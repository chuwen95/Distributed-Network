//
// Created by ChuWen on 9/5/23.
//

#include "SlaveReactor.h"

#include "csm-utilities/Socket.h"
#include "csm-utilities/Logger.h"
#include "csm-utilities/Timestamp.h"

using namespace csm::service;

constexpr std::size_t c_maxEvent{ 500 };

SlaveReactor::SlaveReactor(const int reactorId, const std::string& hostId, TcpSessionManager::Ptr tcpSessionManager) :
        m_reactorId(reactorId), m_tcpSessionManager(std::move(tcpSessionManager))
{
    m_recvBuffer.resize(utilities::Socket::c_defaultSocketRecvBufferSize);
}

SlaveReactor::~SlaveReactor()
{
    utilities::Socket::close(m_epfd);
}

int SlaveReactor::init()
{
    m_epfd = epoll_create1(0);
    if (-1 == m_epfd)
    {
        LOG->write(utilities::LogType::Log_Error, FILE_INFO, "create epoll fd failed, fd: ", m_epfd);
        return -1;
    }
    LOG->write(utilities::LogType::Log_Trace, FILE_INFO, "create epoll fd successfully, fd: ", m_epfd);

    // 创建用于退出的事件套接字
    m_exitFd = eventfd(0, EFD_NONBLOCK);
    if(-1 == m_exitFd)
    {
        LOG->write(utilities::LogType::Log_Error, FILE_INFO, "create eventfd failed, errno: ", errno, ", ", strerror(errno));
        return -1;
    }
    LOG->write(utilities::LogType::Log_Trace, FILE_INFO, "create eventfd successfully, m_exitFd: ", m_exitFd);

    struct epoll_event event;
    event.events = EPOLLIN;
    event.data.fd = m_exitFd;
    if(-1 == epoll_ctl(m_epfd, EPOLL_CTL_ADD, m_exitFd, &event))
    {
        LOG->write(utilities::LogType::Log_Error, FILE_INFO, "add exit event fd to epoll failed, errno: ", errno, ", ", strerror(errno));
        return -1;
    }
    LOG->write(utilities::LogType::Log_Trace, FILE_INFO, "add exit event fd to epoll successfully");

    return 0;
}

int SlaveReactor::start()
{
    const auto expression = [this]()
    {
        static int s_timeout{ 5000 };

        struct epoll_event ev[c_maxEvent];
        int nready = epoll_wait(m_epfd, ev, c_maxEvent, s_timeout);

        if (true == m_isTerminate)
        {
            s_timeout = 1;
            return 0;
        }

        if (-1 == nready)
        {
            LOG->write(utilities::LogType::Log_Error, FILE_INFO, "epoll_wait failed, errno: ", errno, ", ", strerror(errno));
            return -1;
        }
        else if (0 == nready)
        {
        }

        for (int i = 0; i < nready; ++i)
        {
            int fd = ev[i].data.fd;

            // error
            if (ev[i].events & EPOLLERR ||         // 文件描述符发生错误
                ev[i].events & EPOLLHUP)     // 文件描述符被挂断
            {
                LOG->write(utilities::LogType::Log_Error, FILE_INFO, "fd error, fd: ", fd);
                if(nullptr != m_disconnectHandler)
                {
                    // 将客户端从epoll中移除
                    m_disconnectHandler(fd);
                }
                return -1;
            }

            if (ev[i].events & EPOLLIN)
            {
                LOG->write(utilities::LogType::Log_Trace, FILE_INFO, "EPOLLIN event, fd: ", fd);

                int readLen = recv(fd, m_recvBuffer.data(), m_recvBuffer.size(), 0);
                if (readLen < 0)    // 没有数据再需要读取
                {
                }
                else if (0 == readLen)
                {
                    LOG->write(utilities::LogType::Log_Error, FILE_INFO, "client may be offline, fd: ", fd);

                    if(nullptr != m_disconnectHandler)
                    {
                        m_disconnectHandler(fd);
                    }

                    continue;
                }

                if(nullptr != m_sessionDataHandler)
                {
                    m_sessionDataHandler(fd, m_recvBuffer.data(), readLen);
                }
            }
            if (ev[i].events & EPOLLOUT)
            {
                TcpSession::Ptr tcpSession = m_tcpSessionManager->tcpSession(fd);
                if(nullptr == tcpSession)
                {
                    LOG->write(utilities::LogType::Log_Error, FILE_INFO, "find TcpSession by fd failed, fd: ", fd);
                    continue;
                }

                if(true == tcpSession->isWaitingDisconnect())
                {
                    LOG->write(utilities::LogType::Log_Error, FILE_INFO, "TcpSession is waiting for disconnect, fd: ", fd);
                    continue;
                }

                utilities::RingBuffer::Ptr writeBuffer = tcpSession->writeBuffer();
                assert(0 != writeBuffer->dataLength());

                x_writeBuffer.lock();

                char *data{nullptr};
                std::size_t length{0};
                int ret = writeBuffer->getContinuousData(data, length);
                assert(-1 != ret);

                int sendLen = send(fd, data, length, 0);
                if (sendLen == length)
                {
                    writeBuffer->decreaseUsedSpace(sendLen);
                    x_writeBuffer.unlock();

                    struct epoll_event ev;
                    memset(&ev, 0, sizeof(struct epoll_event));
                    ev.events = EPOLLIN | EPOLLET;
                    ev.data.fd = fd;
                    epoll_ctl(m_epfd, EPOLL_CTL_MOD, fd, &ev);

                    LOG->write(utilities::LogType::Log_Debug, FILE_INFO, "send data successfully, remove epollout event, fd: ", fd, ", sendlen: ", sendLen);
                }
                else if(sendLen < length)
                {
                    writeBuffer->decreaseUsedSpace(sendLen);
                    x_writeBuffer.unlock();

                    LOG->write(utilities::LogType::Log_Debug, FILE_INFO, "sendLen less then data length, fd: ", fd, ", sendlen: ", sendLen);
                }
                else if(-1 == sendLen)
                {
                    x_writeBuffer.unlock();

                    if (EAGAIN == errno || EWOULDBLOCK == errno)
                    {
                        // 这种情况理应不会发生
                        LOG->write(utilities::LogType::Log_Debug, FILE_INFO, "sendLen = -1, fd: ", fd, ", errno: ", errno, ", ", strerror(errno));
                    }
                    else if (ECONNRESET == errno || EPIPE == errno)
                    {
                        // 表明对端断开连接
                        LOG->write(utilities::LogType::Log_Error, FILE_INFO, "client maybe offline, fd: ", fd, ", errno: ", errno, ", ", strerror(errno));

                        if(nullptr != m_disconnectHandler)
                        {
                            m_disconnectHandler(fd);
                        }
                    }
                }
            }
        }

        return 0;
    };
    std::string threadName = "slav_reac_" + std::to_string(m_reactorId);
    m_thread.init(expression, 0, threadName.c_str());
    m_thread.start();

    return 0;
}

int SlaveReactor::stop()
{
    m_isTerminate = true;

    std::uint64_t num{ 1 };
    write(m_exitFd, &num, sizeof(std::uint64_t));
    m_thread.stop();

    close(m_exitFd);
    close(m_epfd);

    return 0;
}

int SlaveReactor::addClient(const int fd)
{
    LOG->write(utilities::LogType::Log_Info, FILE_INFO, "SlaveReactor ", m_reactorId, " try to add client ", fd, " to epoll");

    struct epoll_event ev;
    memset(&ev, 0, sizeof(struct epoll_event));
    ev.events = EPOLLIN | EPOLLET;
    ev.data.fd = fd;
    int ret = epoll_ctl(m_epfd, EPOLL_CTL_ADD, fd, &ev);
    if (-1 == ret)
    {
        LOG->write(utilities::LogType::Log_Error, FILE_INFO,
                   "epoll_ctl add failed, fd: ", fd, ", errno: ", errno, ", ", strerror(errno));
        return -1;
    }

    LOG->write(utilities::LogType::Log_Info, FILE_INFO, "SlaveReactor ", m_reactorId,
               " add client ", fd, " to epoll successfully, events: EPOLLIN and EPOLLET", ", m_epfd: ", m_epfd);

    return 0;
}

void SlaveReactor::setDisconnectHandler(const std::function<int(const int)> handler)
{
    m_disconnectHandler = handler;
}

void SlaveReactor::setSessionDataHandler(const std::function<int(const int, const char* data, const std::size_t dataLen)> handler)
{
    m_sessionDataHandler = handler;
}

int SlaveReactor::sendData(const int fd, const char *data, const std::size_t size)
{
    if(size > c_tcpSessionWriteBufferSize)
    {
        LOG->write(utilities::LogType::Log_Error, FILE_INFO, "data size must less than session write buffer size");
        return -1;
    }

    LOG->write(utilities::LogType::Log_Debug, FILE_INFO, "m_id: ", m_reactorId, ", fd: ", fd);
    //获取对应的writeBuffer
    TcpSession::Ptr tcpSession = m_tcpSessionManager->tcpSession(fd);
    if(nullptr == tcpSession)
    {
        LOG->write(utilities::LogType::Log_Error, FILE_INFO, "find TcpSession by fd failed, fd: ", fd);
        return -1;
    }

    if(true == tcpSession->isWaitingDisconnect())
    {
        LOG->write(utilities::LogType::Log_Error, FILE_INFO, "TcpSession is waiting for disconnect, fd: ", fd);
        return -1;
    }

    {
        x_writeBuffer.lock();

        utilities::RingBuffer::Ptr writeBuffer = tcpSession->writeBuffer();
        if(0 == writeBuffer->dataLength())
        {
            int sendLen = send(fd, data, size, 0);
            if (sendLen == size)
            {
                x_writeBuffer.unlock();
                LOG->write(utilities::LogType::Log_Debug, FILE_INFO, "send data successfully, fd: ", fd, ", sendlen: ", sendLen);
                return 0;
            }
            else if(sendLen < size)
            {
                // 拷贝数据到缓冲区，此时缓冲区应该为空，不应该出现返回值为-1放不进去的情况，如果出现，表明代码逻辑错误
                assert(-1 != writeBuffer->writeData(data + sendLen, size - sendLen));
                writeBuffer->increaseUsedSpace(size - sendLen);

                x_writeBuffer.unlock();

                struct epoll_event ev;
                memset(&ev, 0, sizeof(struct epoll_event));
                ev.events = EPOLLIN | EPOLLOUT | EPOLLET;
                ev.data.fd = fd;
                epoll_ctl(m_epfd, EPOLL_CTL_MOD, fd, &ev);

                LOG->write(utilities::LogType::Log_Debug, FILE_INFO,
                           "sendLen < size, write remain data to buffer successfully and wait for epollout, remain data size: ", size - sendLen, ", fd: ", fd);
            }
            else if(-1 == sendLen)
            {
                x_writeBuffer.unlock();

                LOG->write(utilities::LogType::Log_Debug, FILE_INFO, "send data successfully, fd: ", fd, ", sendlen: ", sendLen);
                // 如果发送缓冲区中还有数据，但数据直接send完，表明无需等待EPOLLOUT事件
                if (EAGAIN == errno || EWOULDBLOCK == errno)
                {
                    struct epoll_event ev;
                    memset(&ev, 0, sizeof(struct epoll_event));
                    ev.events = EPOLLIN | EPOLLOUT | EPOLLET;
                    ev.data.fd = fd;
                    epoll_ctl(m_epfd, EPOLL_CTL_MOD, fd, &ev);

                    LOG->write(utilities::LogType::Log_Debug, FILE_INFO, "socket write kernel buffer full, wait for epollout, fd: ", fd);
                }
                else if (ECONNRESET == errno || EPIPE == errno)
                {
                    // 表明对端断开连接
                    LOG->write(utilities::LogType::Log_Error, FILE_INFO, "client maybe offline, fd: ", fd, ", errno: ", errno, ", ", strerror(errno));

                    if(nullptr != m_disconnectHandler)
                    {
                        m_disconnectHandler(fd);
                    }
                }
            }
        }
    }

    return 0;
}

int SlaveReactor::sendData(const int fd, const std::vector<char> data)
{
    return sendData(fd, data.data(), data.size());
}

int SlaveReactor::removeClient(const int fd)
{
    // 将客户端从epoll中移除
    return epoll_ctl(m_epfd, EPOLL_CTL_DEL, fd, nullptr);
}
#if 0
int SlaveReactor::disconnectClient(const int fd, const int flag)
{
    TcpSession::Ptr tcpSession = m_tcpSessionManager->tcpSession(fd);
    if (nullptr == tcpSession)
    {
        LOG->write(utilities::LogType::Log_Info, FILE_INFO, "client not exist, fd: ", fd);
        return -1;
    }

    // 将客户端从在线监测中移除
    m_clientAliveChecker.removeClient(fd);

    // 关闭客户端fd
    close(fd);

    if (nullptr != m_disconnectHandler)
    {
        m_disconnectHandler(fd, tcpSession->peerHostEndPointInfo(), tcpSession->getClientId(), tcpSession->handshakeUuid(), flag);
    }
    m_tcpSessionManager->removeTcpSession(fd);

    LOG->write(utilities::LogType::Log_Info, FILE_INFO, "client disconnect finish, fd: ", fd);

    return 0;
}
#endif