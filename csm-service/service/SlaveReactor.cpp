//
// Created by ChuWen on 9/5/23.
//

#include "SlaveReactor.h"

#include <shared_mutex>
#include <cstring>
#include <cassert>

#include <sys/epoll.h>
#include <sys/eventfd.h>
#include <netinet/in.h>
#include <netinet/tcp.h>

#include "csm-utilities/Logger.h"
#include "csm-utilities/Socket.h"

using namespace csm::service;

constexpr std::size_t c_maxEvent{500};
constexpr int c_epollTimeout{500};

SlaveReactor::SlaveReactor(const int reactorId) : m_reactorId(reactorId)
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
    if (-1 == m_exitFd)
    {
        LOG->write(utilities::LogType::Log_Error, FILE_INFO, "create eventfd failed, errno: ", errno, ", ",
                   strerror(errno));
        return -1;
    }
    LOG->write(utilities::LogType::Log_Trace, FILE_INFO, "create eventfd successfully, m_exitFd: ", m_exitFd);

    struct epoll_event event;
    event.events = EPOLLIN;
    if (-1 == epoll_ctl(m_epfd, EPOLL_CTL_ADD, m_exitFd, &event))
    {
        LOG->write(utilities::LogType::Log_Error, FILE_INFO, "add exit event fd to epoll failed, errno: ", errno, ", ",
                   strerror(errno));
        return -1;
    }
    LOG->write(utilities::LogType::Log_Trace, FILE_INFO, "add exit event fd to epoll successfully");

    const auto expression = [this](const std::stop_token& st)
    {
        struct epoll_event ev[c_maxEvent];
        int nready = epoll_wait(m_epfd, ev, c_maxEvent, -1);

        if (true == st.stop_requested())
        {
            return 0;
        }

        if (-1 == nready)
        {
            LOG->write(utilities::LogType::Log_Error, FILE_INFO, "epoll_wait failed, errno: ", errno, ", ",
                       strerror(errno));
            return -1;
        }
        else if (0 == nready)
        {
        }

        for (int i = 0; i < nready; ++i)
        {
            SessionId sessionId = ev[i].data.u64;
            P2PSession::Ptr p2pSession = getP2PSession(sessionId);
            if (nullptr == p2pSession)
            {
                LOG->write(utilities::LogType::Log_Error, FILE_INFO, "client may be offline, session id: ", sessionId);
                continue;;
            }

            // error
            if (ev[i].events & EPOLLERR || // 文件描述符发生错误
                ev[i].events & EPOLLHUP) // 文件描述符被挂断
            {
                disconnectSession(p2pSession);
                continue;
            }

            if (ev[i].events & EPOLLIN)
            {
                LOG->write(utilities::LogType::Log_Trace, FILE_INFO, "EPOLLIN event, session id: ", sessionId);

                int readLen = recv(p2pSession->fd(), m_recvBuffer.data(), m_recvBuffer.size(), 0);
                if (readLen < 0) // 没有数据再需要读取
                {
                    LOG->write(utilities::LogType::Log_Error, FILE_INFO,
                               "client may be offline, readlen: -1, session id: ",
                               sessionId);

                    disconnectSession(p2pSession);
                    continue;
                }
                else if (0 == readLen)
                {
                    LOG->write(utilities::LogType::Log_Error, FILE_INFO,
                               "client may be offline, readlen: 0, session id: ",
                               sessionId);

                    disconnectSession(p2pSession);
                    continue;
                }

                if (nullptr != m_sessionDataHandler)
                {
                    m_sessionDataHandler(sessionId, p2pSession, m_recvBuffer.data(), readLen);
                }
            }
            if (ev[i].events & EPOLLOUT)
            {
                if (true == p2pSession->isWaitingDisconnect())
                {
                    LOG->write(utilities::LogType::Log_Error, FILE_INFO,
                               "P2PSession is waiting for disconnect, session id: ",
                               sessionId);
                    continue;
                }

                utilities::RingBuffer* writeBuffer = p2pSession->writeBuffer();
                assert(0 != writeBuffer->dataLength());

                x_writeBuffer.lock();

                char* data{nullptr};
                std::size_t length{0};
                int ret = writeBuffer->getContinuousData(data, length);
                assert(-1 != ret);

                int sendLen = send(p2pSession->fd(), data, length, 0);
                if (sendLen == length)
                {
                    writeBuffer->decreaseUsedSpace(sendLen);
                    x_writeBuffer.unlock();

                    struct epoll_event ev;
                    memset(&ev, 0, sizeof(struct epoll_event));
                    ev.events = EPOLLIN | EPOLLET;
                    ev.data.u64 = sessionId;
                    epoll_ctl(m_epfd, EPOLL_CTL_MOD, p2pSession->fd(), &ev);

                    LOG->write(utilities::LogType::Log_Debug, FILE_INFO,
                               "send data successfully, remove epollout event, session id: ", sessionId, ", sendlen: ",
                               sendLen);
                }
                else if (sendLen < length)
                {
                    writeBuffer->decreaseUsedSpace(sendLen);
                    x_writeBuffer.unlock();

                    LOG->write(utilities::LogType::Log_Debug, FILE_INFO, "sendLen less then data length, session id: ",
                               sessionId,
                               ", sendlen: ", sendLen);
                }
                else if (-1 == sendLen)
                {
                    x_writeBuffer.unlock();

                    if (EAGAIN == errno || EWOULDBLOCK == errno)
                    {
                        // 这种情况理应不会发生
                        LOG->write(utilities::LogType::Log_Debug, FILE_INFO, "sendLen = -1, session id: ", sessionId,
                                   ", errno: ",
                                   errno, ", ",
                                   strerror(errno));
                    }
                    else if (ECONNRESET == errno || EPIPE == errno)
                    {
                        // 表明对端断开连接
                        LOG->write(utilities::LogType::Log_Error, FILE_INFO, "client maybe offline, session id: ",
                                   sessionId,
                                   ", errno: ", errno,
                                   ", ", strerror(errno));

                        disconnectSession(p2pSession);
                    }
                }
            }
        }

        return 0;
    };
    m_thread = std::make_unique<utilities::Thread>(expression, 0, "slav_reac_" + std::to_string(m_reactorId));

    return 0;
}

int SlaveReactor::start()
{
    m_thread->start();

    return 0;
}

int SlaveReactor::stop()
{
    m_thread->stop();

    std::uint64_t num{1};
    write(m_exitFd, &num, sizeof(std::uint64_t));

    close(m_exitFd);
    close(m_epfd);

    return 0;
}

int SlaveReactor::addSession(const int fd, P2PSession::Ptr p2pSession)
{
    LOG->write(utilities::LogType::Log_Info, FILE_INFO, "SlaveReactor ", m_reactorId,
               " try to add session to epoll, fd: ",
               fd, ", session id: ", p2pSession->sessionId());

    struct epoll_event ev;
    memset(&ev, 0, sizeof(struct epoll_event));
    ev.events = EPOLLIN | EPOLLET;
    ev.data.u64 = p2pSession->sessionId();
    int ret = epoll_ctl(m_epfd, EPOLL_CTL_ADD, fd, &ev);
    if (-1 == ret)
    {
        LOG->write(utilities::LogType::Log_Error, FILE_INFO, "epoll_ctl add failed, fd: ", fd, ", session id: ",
                   p2pSession->sessionId(), ", errno: ", errno, ", ",
                   strerror(errno));
        return -1;
    }

    std::unique_lock<std::shared_mutex> wlock(x_p2pSessions);
    m_p2pSessions.emplace(p2pSession->sessionId(), p2pSession);

    LOG->write(utilities::LogType::Log_Info, FILE_INFO, "SlaveReactor ", m_reactorId,
               " epoll_ctl add successfully, events: EPOLLIN | EPOLLET, m_epfd: ", m_epfd, ", fd: ", fd,
               ", session id: ",
               p2pSession->sessionId());

    return 0;
}

void SlaveReactor::setDisconnectHandler(
    std::function<void(SessionId sessionId, P2PSession::WPtr p2pSessionWeakPtr)> handler)
{
    m_disconnectHandler = std::move(handler);
}

void SlaveReactor::setSessionDataHandler(
    std::function<void(SessionId sessionId, P2PSession::WPtr p2pSessionWeakPtr, const char* data,
                       std::size_t dataLen)> handler)
{
    m_sessionDataHandler = std::move(handler);
}

int SlaveReactor::sendData(const SessionId sessionId, const std::vector<char>& data)
{
    LOG->write(utilities::LogType::Log_Debug, FILE_INFO, "m_id: ", m_reactorId, ", session id: ", sessionId);
    // 获取对应的writeBuffer
    P2PSession::Ptr p2pSession = getP2PSession(sessionId);
    if (nullptr == p2pSession)
    {
        LOG->write(utilities::LogType::Log_Error, FILE_INFO, "find P2PSession by fd failed, session id: ", sessionId);
        return -1;
    }

    return sendData(p2pSession, data);
}

int SlaveReactor::sendData(P2PSession::Ptr p2pSession, const std::vector<char>& data)
{
    if (data.size() > c_p2pSessionWriteBufferSize)
    {
        LOG->write(utilities::LogType::Log_Error, FILE_INFO, "data size must less than session write buffer size");
        return -1;
    }

    if (true == p2pSession->isWaitingDisconnect())
    {
        LOG->write(utilities::LogType::Log_Error, FILE_INFO, "P2PSession is waiting for disconnect, session id: ",
                   p2pSession->sessionId());
        return -1;
    }

    {
        x_writeBuffer.lock();

        utilities::RingBuffer* writeBuffer = p2pSession->writeBuffer();
        if (0 == writeBuffer->dataLength())
        {
            int sendLen = send(p2pSession->fd(), data.data(), data.size(), 0);
            if (sendLen == data.size())
            {
                x_writeBuffer.unlock();
                LOG->write(utilities::LogType::Log_Debug, FILE_INFO, "send data successfully, session id: ",
                           p2pSession->sessionId(),
                           ", sendlen: ", sendLen);
                return 0;
            }
            else if (sendLen < data.size())
            {
                // 拷贝数据到缓冲区，此时缓冲区应该为空，不应该出现返回值为-1放不进去的情况，如果出现，表明代码逻辑错误
                assert(-1 != writeBuffer->writeData(data.data() + sendLen, data.size() - sendLen));
                writeBuffer->increaseUsedSpace(data.size() - sendLen);

                x_writeBuffer.unlock();

                struct epoll_event ev;
                memset(&ev, 0, sizeof(struct epoll_event));
                ev.events = EPOLLIN | EPOLLOUT | EPOLLET;
                ev.data.u64 = p2pSession->sessionId();
                epoll_ctl(m_epfd, EPOLL_CTL_MOD, p2pSession->fd(), &ev);

                LOG->write(utilities::LogType::Log_Debug, FILE_INFO,
                           "sendLen < size, write remain data to buffer successfully and wait for epollout, remain data size: ",
                           data.size() - sendLen, ", session id: ", p2pSession->sessionId());
            }
            else if (-1 == sendLen)
            {
                x_writeBuffer.unlock();

                LOG->write(utilities::LogType::Log_Debug, FILE_INFO, "send data successfully, session id: ",
                           p2pSession->sessionId(),
                           ", sendlen: ", sendLen);
                // 如果发送缓冲区中还有数据，但数据直接send完，表明无需等待EPOLLOUT事件
                if (EAGAIN == errno || EWOULDBLOCK == errno)
                [[likely]]
                {
                    struct epoll_event ev;
                    memset(&ev, 0, sizeof(struct epoll_event));
                    ev.events = EPOLLIN | EPOLLOUT | EPOLLET;
                    ev.data.fd = p2pSession->sessionId();
                    epoll_ctl(m_epfd, EPOLL_CTL_MOD, p2pSession->fd(), &ev);

                    LOG->write(utilities::LogType::Log_Debug, FILE_INFO,
                               "socket write kernel buffer full, wait for epollout, session id: ",
                               p2pSession->sessionId());
                }
                else if (ECONNRESET == errno || EPIPE == errno)
                [[unlikely]]
                {
                    // 表明对端断开连接
                    LOG->write(utilities::LogType::Log_Error, FILE_INFO, "client maybe offline, session id: ",
                               p2pSession->sessionId(),
                               ", errno: ", errno,
                               ", ", strerror(errno));

                    if (nullptr != m_disconnectHandler) [[likely]]
                    {
                        LOG->write(utilities::LogType::Log_Info, FILE_INFO, "add fd to session destoryer, session id: ",
                                   p2pSession->sessionId());
                        m_disconnectHandler(p2pSession->sessionId(), p2pSession);
                    }
                }
            }
        }
    }

    return 0;
}

int SlaveReactor::removeClient(const SessionId sessionId, const int fd)
{
    // 将客户端从epoll中移除
    if (-1 == epoll_ctl(m_epfd, EPOLL_CTL_DEL, fd, nullptr))
    {
        LOG->write(utilities::LogType::Log_Error, FILE_INFO, "remove fd from epoll failed, session id: ", sessionId,
                   ", fd: ", fd, ", errno: ", errno, ", ", strerror(errno));
        return -1;
    }

    std::unique_lock<std::shared_mutex> wlock(x_p2pSessions);
    m_p2pSessions.erase(sessionId);

    return 0;
}

P2PSession::Ptr SlaveReactor::getP2PSession(const SessionId sessionId)
{
    std::shared_lock<std::shared_mutex> rlock(x_p2pSessions);

    auto iter = m_p2pSessions.find(sessionId);
    if (m_p2pSessions.end() == iter)
    {
        return nullptr;
    }

    return iter->second;
}

void SlaveReactor::disconnectSession(const P2PSession::Ptr& p2pSession)
{
    p2pSession->setWaitingDisconnect(true);

    if (nullptr != m_disconnectHandler)
    {
        P2PSession::WPtr p2pSessionWeakPtr = p2pSession;
        m_disconnectHandler(p2pSession->sessionId(), p2pSessionWeakPtr);
    }
}
