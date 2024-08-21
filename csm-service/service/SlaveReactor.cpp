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
    m_reactorId(reactorId), m_hostId(hostId), m_tcpSessionManager(std::move(tcpSessionManager))
{
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
    LOG->write(utilities::LogType::Log_Debug, FILE_INFO, "create epoll fd successfully, fd: ", m_epfd);

    m_clientAliveChecker.init([this](const std::vector<int> &fds) {
        onClientsHeartbeatTimeout(fds);
    });

    return 0;
}

int SlaveReactor::start()
{
    const auto expression = [this]()
    {
        if (true == m_isTerminate)
        {
            std::this_thread::sleep_for(std::chrono::milliseconds(5));
            return 0;
        }

        int timeout{0};
        if (true == m_infds.empty() && true == m_outfds.empty() && true == m_datafds.empty())
        {
            timeout = 100;
        }
        struct epoll_event ev[c_maxEvent];
        int nready = epoll_wait(m_epfd, ev, c_maxEvent, timeout);
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
                disconnectClient(fd);
                return -1;
            }

            if (ev[i].events & EPOLLIN)
            {
                LOG->write(utilities::LogType::Log_Trace, FILE_INFO, "EPOLLIN event, fd: ", fd);
                {
                    if(false == m_tcpSessionManager->isTcpSessionExist(fd))
                    {
                        LOG->write(utilities::LogType::Log_Error, FILE_INFO, "fd not found, fd: ", fd);
                        continue;
                    }
                    m_infds.insert(fd);
                }
            }
            if (ev[i].events & EPOLLOUT)
            {
                TcpSession::Ptr tcpSession = m_tcpSessionManager->tcpSession(fd);
                if(nullptr == tcpSession)
                {
                    LOG->write(utilities::LogType::Log_Error, FILE_INFO, "fd not found, fd: ", fd);
                    continue;
                }

                if (0 == tcpSession->writeBuffer()->dataLength())
                {
                    continue;
                }

                std::unique_lock<std::mutex> ulock(x_outfds);

                auto iter = m_outfds.find(fd);
                if (m_outfds.end() != iter)
                {
                    iter->second.second = true;
                }
            }
        }

        // 处理fd读事件
        for (auto iter = m_infds.begin(); iter != m_infds.end();)
        {
            int fd = *iter;

            TcpSession::Ptr tcpSession = m_tcpSessionManager->tcpSession(fd);
            if(nullptr == tcpSession)
            {
                LOG->write(utilities::LogType::Log_Error, FILE_INFO, "fd not found, fd: ", fd);
                iter = m_infds.erase(iter);
                continue;
            }

            utilities::RingBuffer::Ptr readBuffer = tcpSession->readBuffer();

            // 如果缓冲区满了，则无法在接收数据，继续外层for循环，但是由于边缘触发的原因，不能将fd从m_infds中移除
            char *data{nullptr};
            std::size_t length{0};
            int ret = readBuffer->getBufferAndLengthForWrite(data, length);
            if (-1 == ret)
            {
                LOG->write(utilities::LogType::Log_Debug, FILE_INFO, "getBufferAndLengthForWrite failed, fd: ", fd);
                // 致命错误，缓冲区使用方法不正确
                assert(-1 != ret);
            }
            LOG->write(utilities::LogType::Log_Debug, FILE_INFO, "getBufferAndLengthForWrite, length:", length, ", fd: ", fd);

            int readLen = recv(fd, data, length, 0);
            if (readLen < 0)
            {
                // 没有数据再需要读取，从m_infds中移除
                iter = m_infds.erase(iter);
                continue;
            }
            else if (0 == readLen)
            {
                LOG->write(utilities::LogType::Log_Error, FILE_INFO, "client may be offline, fd: ", fd);
                iter = m_infds.erase(iter);
                disconnectClient(fd);
                continue;
            }
            // 增加已经使用的空间
            ret = readBuffer->increaseUsedSpace(readLen);
            if (-1 == ret)
            {
                LOG->write(utilities::LogType::Log_Debug, FILE_INFO,
                              "increaseUsedSpace failed, space: ", readBuffer->space(), ", readLen: ", readLen, ", fd: ", fd);
                // 致命错误，缓冲区使用方法不正确
                assert(-1 != ret);
            }

            LOG->write(utilities::LogType::Log_Debug, FILE_INFO,
                          "refresh last recv time, recv length: ", readLen, ", readBuffer dataLength: ", readBuffer->dataLength(), ", fd: ", fd);

            // 更新时间戳
            m_clientAliveChecker.refreshClientLastRecvTime(fd);

            // 接收到了数据，需要回调给业务层
            m_datafds.insert(fd);

            ++iter;
        }

        // 接收到了数据，需要回调给业务层
        for (auto iter = m_datafds.begin(); iter != m_datafds.end();)
        {
            int fd = *iter;
            LOG->write(utilities::LogType::Log_Trace, FILE_INFO, "try to decode packet, fd: ", fd);

            TcpSession::Ptr tcpSession = m_tcpSessionManager->tcpSession(fd);
            if(nullptr == tcpSession)
            {
                LOG->write(utilities::LogType::Log_Error, FILE_INFO, "fd not found, fd: ", fd);
                iter = m_datafds.erase(iter);
                continue;
            }

            utilities::RingBuffer::Ptr readBuffer = tcpSession->readBuffer();
            if (0 == readBuffer->dataLength())
            {
                LOG->write(utilities::LogType::Log_Error, FILE_INFO, "fd: ", fd);
                assert(0 != readBuffer->dataLength());
            }

#if 0
            utilities::Timestamp timestamp;
            timestamp.update();
#endif
            std::size_t epochPacketSum{100};
            std::size_t packetNum{0};
            for (packetNum = 0; packetNum < epochPacketSum; ++packetNum)
            {
                LOG->write(utilities::LogType::Log_Debug, FILE_INFO, "readBuffer length: ", readBuffer->dataLength(),
                              ", startOffset: ", readBuffer->startOffset(), ", endOffset: ", readBuffer->endOffset(), ", fd: ", fd);

                PacketType packetType{PacketType::PT_None};
                std::int32_t moduleId{-1};
                std::shared_ptr<std::vector<char>> packetPayload = std::make_shared<std::vector<char>>();
                int ret = getPacket(fd, readBuffer, packetType, moduleId, packetPayload);
                if (0 != ret)
                {
                    // 缓冲区中的数据已经不够一个包头的长度
                    LOG->write(utilities::LogType::Log_Debug, FILE_INFO, "pop packet finish", ", fd: ", fd);
                    break;
                }

                if (PacketType::PT_None == packetType)
                {
                    LOG->write(utilities::LogType::Log_Error, FILE_INFO, "decode packet failed, fd: ", fd);
                }
                else if (PacketType::PT_ClientInfo == packetType)
                {
                    // 身份包，即刻处理，不必放到线程池中
                    LOG->write(utilities::LogType::Log_Info, FILE_INFO, "recv ClientInfo packet, fd: ", fd);
                    PacketClientInfo::Ptr clientInfoPacket = std::make_shared<PacketClientInfo>(packetPayload);
                    if (-1 == processClientInfoPacket(fd, tcpSession, clientInfoPacket))
                    {
                        LOG->write(utilities::LogType::Log_Error, FILE_INFO, "process ClientInfo packet failed, fd: ", fd);
                        // 将packetNum置为小于epochPacketSum，后续会将fd从m_datafds中移除
                        packetNum = 0;
                        // 从m_infds中移除，因为m_infds中的socket如果没有数据可读了或者断开了，再下一次循环中才会发现并移除
                        m_infds.erase(fd);
                    }
                }
                else if (PacketType::PT_ClientInfoReply == packetType)
                {
                    // 身份包，即刻处理，不必放到线程池中
                    LOG->write(utilities::LogType::Log_Info, FILE_INFO, "recv ClientInfoReply packet, fd: ", fd);
                    PacketClientInfoReply::Ptr clientInfoReplyPacket = std::make_shared<PacketClientInfoReply>(packetPayload);
                    int ret = processClientInfoReplyPacket(fd, tcpSession, clientInfoReplyPacket);
                    if (0 != ret)
                    {
                        LOG->write(utilities::LogType::Log_Error, FILE_INFO, "process ClientInfoReply packet failed, ret: ", ret, ", fd: ", fd);
                        if (-1 == ret)
                        {
                            // 将packetNum置为小于epochPacketSum，后续会将fd从m_datafds中移除
                            packetNum = 0;
                            // 从m_infds中移除，因为m_infds中的socket如果没有数据可读了或者断开了，再下一次循环中才会发现并移除
                            m_infds.erase(fd);
                        }
                    }
                }
                else if (PacketType::PT_HeartBeat == packetType || PacketType::PT_HeartBeatReply == packetType)
                {
                    // 心跳包，不必放到线程池中处理
                    LOG->write(utilities::LogType::Log_Trace, FILE_INFO, "receive heartbeat packet, fd: ", fd);
                    m_clientAliveChecker.refreshClientLastRecvTime(fd);
                }
                else
                {
                    LOG->write(utilities::LogType::Log_Debug, FILE_INFO,
                                  "callback packet type and packet payload, fd: ", fd, ", packet type: ", static_cast<int>(packetType));
                    if (nullptr != m_moduleMessageHandler)
                    {
                        m_moduleMessageHandler(fd, moduleId, packetPayload);
                    }
                }
            }
            // 1. 如果刚好取出epochPacketNum后缓冲区为空了，packetNum会等于epochPacketSum，需要判断下缓冲区是否还有数据
            // 2. 如果packetNum < epochPacketSum，那肯定是缓冲区数据不够一个包了，直接从m_datafds中移除
            if (0 == readBuffer->dataLength() || packetNum < epochPacketSum)
            {
                iter = m_datafds.erase(iter);
            }
            else
            {
                ++iter;
            }
            LOG->write(utilities::LogType::Log_Debug, FILE_INFO, "decode packet finish, fd: ", fd, ", packet num: ", packetNum);
#if 0
            LOG->write(utilities::LogType::Log_Debug, FILE_INFO,
                                                                         "elapsed time: ", timestamp.getElapsedTimeInMilliSec());
#endif
        }

        // 处理fd写
        std::unordered_map<int, std::pair<bool, bool>> outfds;
        {
            std::unique_lock<std::mutex> ulock(x_outfds);
            outfds = m_outfds;
        }
        for (auto iter = outfds.begin(); iter != outfds.end();)
        {
            int fd = iter->first;

            TcpSession::Ptr tcpSession = m_tcpSessionManager->tcpSession(fd);
            if(nullptr == tcpSession)
            {
                LOG->write(utilities::LogType::Log_Error, FILE_INFO, "fd not found, fd: ", fd);
                iter = outfds.erase(iter);
                continue;
            }

            if (true == iter->second.first && false == iter->second.second)
            {
                // 如果fd正在等待EPOLLOUT事件且EPOLLOUT事件没来
                ++iter;
                continue;
            }

            utilities::RingBuffer::Ptr writeBuffer = tcpSession->writeBuffer();
            if (0 == writeBuffer->dataLength())
            {
                iter = outfds.erase(iter);
                continue;
            }

            {
                std::unique_lock<std::mutex> ulock(x_writeBuffer);

                char *data{nullptr};
                std::size_t length{0};
                int ret = writeBuffer->getContinuousData(data, length);
                assert(-1 != ret);

                int sendLen = send(fd, data, length, 0);
                if (sendLen == length)
                {
                    writeBuffer->decreaseUsedSpace(sendLen);
                    LOG->write(utilities::LogType::Log_Debug, FILE_INFO, "send data successfully, fd: ", fd, ", sendlen: ", sendLen);
                    if (0 == writeBuffer->dataLength())
                    {
                        // 如果发送缓冲区中数据都没有了，直接从outfds中移除fd
                        iter = outfds.erase(iter);
                        continue;
                    } else
                    {
                        // 如果发送缓冲区中还有数据，但数据直接send完，表明无需等待EPOLLOUT事件
                        iter->second.first = false;
                    }
                }
                else
                {
                    if (-1 != sendLen && (EAGAIN == errno || EWOULDBLOCK == errno))
                    {
                        // 还有没发出去的数据，或者说没法放到发送缓冲区的数据
                        writeBuffer->decreaseUsedSpace(sendLen);
                        // 设置fd正在等待EPOLLOUT事件为true
                        iter->second.first = true;
                        // 设置EPOLLOUT事件到来为false
                        iter->second.second = false;
                        LOG->write(utilities::LogType::Log_Debug, FILE_INFO, "send data error, fd: ", fd, ", errno: ", errno, ", ", strerror(errno));
                    }
                    else if (ECONNRESET == errno || EPIPE == errno)
                    {
                        // 表明对端断开连接
                        LOG->write(utilities::LogType::Log_Error, FILE_INFO, "client maybe offline, fd: ", fd, ", errno: ", errno, ", ", strerror(errno));

                        iter = outfds.erase(iter);
                        disconnectClient(fd);

                        continue;
                    }
                }
            }
            ++iter;
        }

        {
            std::unique_lock<std::mutex> ulock(x_outfds);
            m_outfds = outfds;
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
    m_thread.stop();
    m_thread.uninit();

    return 0;
}

int SlaveReactor::addClient(const int fd)
{
    LOG->write(utilities::LogType::Log_Info, FILE_INFO, "SlaveReactor ", m_reactorId, " try to add client ", fd, " to epoll");

    struct epoll_event ev;
    memset(&ev, 0, sizeof(struct epoll_event));
    ev.events = EPOLLIN | EPOLLOUT | EPOLLET;
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

void SlaveReactor::registerClientInfoHandler(std::function<int(const HostEndPointInfo &, const HostEndPointInfo &,
                                                               const int, const std::string &, const std::string &)> clientInfoHandler)
{
    m_clientInfoHandler = std::move(clientInfoHandler);
}

void SlaveReactor::registerClientInfoReplyHandler(std::function<int(const HostEndPointInfo &, const int, const std::string &,
                                                               const std::string &, const int, int &)> clientInfoReplyHandler)
{
    m_clientInfoReplyHandler = std::move(clientInfoReplyHandler);
}

void SlaveReactor::registerModuleMessageHandler(std::function<void(const int, const std::int32_t,
                                                                   std::shared_ptr<std::vector<char>> &)> moduleMessageHandler)
{
    m_moduleMessageHandler = std::move(moduleMessageHandler);
}

void SlaveReactor::registerDisconnectHandler(std::function<void(const int fd, const HostEndPointInfo &hostEndPointInfo,
                                                                const std::string &id, const std::string &uuid, const int flag)> disconnectHandler)
{
    m_disconnectHandler = std::move(disconnectHandler);
}

int SlaveReactor::sendData(const int fd, const char *data, const std::size_t size)
{
    LOG->write(utilities::LogType::Log_Debug, FILE_INFO, "m_id: ", m_reactorId, ", fd: ", fd);
    //获取对应的writeBuffer
    TcpSession::Ptr tcpSession = m_tcpSessionManager->tcpSession(fd);
    if(nullptr == tcpSession)
    {
        LOG->write(utilities::LogType::Log_Error, FILE_INFO, "id not found, id: ", fd);
        return -2;
    }

    {
        std::unique_lock<std::mutex> ulock(x_writeBuffer);
        utilities::RingBuffer::Ptr writeBuffer = tcpSession->writeBuffer();

        // 拷贝数据到缓冲区
        if (-1 == writeBuffer->writeData(data, size))
        {
            // 缓冲区放不下，返回错误给业务层
            LOG->write(utilities::LogType::Log_Error, FILE_INFO, "buffer not enough, fd: ", fd);
            return -1;
        }
        writeBuffer->increaseUsedSpace(size);
        LOG->write(utilities::LogType::Log_Debug, FILE_INFO, "write data to buffer successfully, size: ", size, ", fd: ", fd);
    }

    // fd有数据发送
    std::unique_lock<std::mutex> ulock(x_outfds);
    if (m_outfds.end() == m_outfds.find(tcpSession->fd()))
    {
        m_outfds.emplace(tcpSession->fd(), std::make_pair(false, false));
    }

    return 0;
}

int SlaveReactor::removeClient(const int fd)
{
    // 将客户端从epoll中移除
    epoll_ctl(m_epfd, EPOLL_CTL_DEL, fd, nullptr);
}

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

int SlaveReactor::getPacket(const int fd, utilities::RingBuffer::Ptr &readBuffer, PacketType &packetType,
                            std::int32_t &moduleId, std::shared_ptr<std::vector<char>> &data)
{
    PacketHeader packetHeader;
    const std::size_t headerLength = packetHeader.headerLength();

    LOG->write(utilities::LogType::Log_Debug, FILE_INFO, "readBuffer->length(): ", readBuffer->dataLength());
    if (readBuffer->dataLength() < headerLength)
    {
        LOG->write(utilities::LogType::Log_Debug, FILE_INFO, "buffer data length less than header length, fd: ", fd);
        return -1;
    }

    char *buffer{nullptr};
    std::vector<char> bufferForBackspaceLessThanHeaderLength;

    int ret = readBuffer->getBufferForRead(headerLength, buffer);
    if (-2 == ret)
    {
        bufferForBackspaceLessThanHeaderLength.resize(headerLength);
        readBuffer->readData(headerLength, 0, bufferForBackspaceLessThanHeaderLength);
        buffer = bufferForBackspaceLessThanHeaderLength.data();
    } else if (-1 == ret)
    {
        return -1;
    }

    packetHeader.decode(buffer, headerLength);
    assert(true == packetHeader.isMagicMatch());
    packetType = packetHeader.type();
    moduleId = packetHeader.moduleId();

    LOG->write(utilities::LogType::Log_Debug, FILE_INFO,
                  "decode packet header successfully, packet type: ", static_cast<int>(packetType),
                  ", payload length: ", packetHeader.payloadLength(), ", fd: ", fd);

    // 尝试复制包数据
    std::size_t payloadLength = packetHeader.payloadLength();
    if (readBuffer->dataLength() - headerLength < payloadLength)
    {
        LOG->write(utilities::LogType::Log_Trace, FILE_INFO,
                      "payload length not enough, packet type: ", static_cast<int>(packetType), " fd: ", fd);
        return -1;
    }

    data->resize(payloadLength);
    if (-1 == (ret = readBuffer->readData(payloadLength, headerLength, *data)))
    {
        LOG->write(utilities::LogType::Log_Trace, FILE_INFO,
                      "payload length not enough, packet type: ", static_cast<int>(packetType), " fd: ", fd);
        assert(-1 != ret);
    }

    readBuffer->decreaseUsedSpace(headerLength + payloadLength);

    LOG->write(utilities::LogType::Log_Debug, FILE_INFO, "readBuffer dataLength: ", readBuffer->dataLength(), " fd: ", fd);

    return 0;
}

int SlaveReactor::processClientInfoPacket(const int fd, TcpSession::Ptr tcpSession, PacketClientInfo::Ptr packetClientInfo)
{
    if (nullptr == packetClientInfo)
    {
        LOG->write(utilities::LogType::Log_Error, FILE_INFO, "decode ClientInfo packet failed", ", fd: ", fd);
        return -1;
    }

    std::string id = packetClientInfo->nodeId();
    tcpSession->setClientId(id);
    tcpSession->setHandshakeUuid(packetClientInfo->handshakeUuid());
    tcpSession->setClientOnlineTimestamp(utilities::Timestamp::getCurrentTimestamp());

    if (nullptr != m_clientInfoHandler)
    {
        int ret = m_clientInfoHandler(packetClientInfo->localHost(), packetClientInfo->peerHost(), fd, id, packetClientInfo->handshakeUuid());
        if (0 != ret)
        {
            // 处理ClientInfoPacket失败
            disconnectClient(fd);
            return ret;
        }
    }

    return 0;
}

int SlaveReactor::processClientInfoReplyPacket(const int fd, TcpSession::Ptr tcpSession, PacketClientInfoReply::Ptr packetClientInfoReply)
{
    if (nullptr == packetClientInfoReply)
    {
        LOG->write(utilities::LogType::Log_Error, FILE_INFO, "decode ClientInfo packet failed", ", fd: ", fd);
        return -1;
    }

    tcpSession->setClientId(packetClientInfoReply->nodeId());
    tcpSession->setClientOnlineTimestamp(utilities::Timestamp::getCurrentTimestamp());

    if (nullptr != m_clientInfoReplyHandler)
    {
        int anotherConnectionFd{-1};
        int ret = m_clientInfoReplyHandler(tcpSession->peerHostEndPointInfo(), fd, packetClientInfoReply->nodeId(),
                                           packetClientInfoReply->handshakeUuid(), packetClientInfoReply->result(),
                                           anotherConnectionFd);
        if (0 != ret)
        {
            if (-3 == packetClientInfoReply->result())
            {
                LOG->write(utilities::LogType::Log_Error, FILE_INFO,
                              "another connection fd: ", anotherConnectionFd, ", peer host: ", packetClientInfoReply->peerHost());
                assert(-1 != anotherConnectionFd);

                TcpSession::Ptr anotherTcpSession = m_tcpSessionManager->tcpSession(anotherConnectionFd);
                assert(nullptr != anotherTcpSession);
                anotherTcpSession->setPeerHostEndPointInfo(packetClientInfoReply->peerHost());

                LOG->write(utilities::LogType::Log_Error, FILE_INFO,
                              "another connection fd: ", anotherConnectionFd, ", peer host: ", anotherTcpSession->peerHostEndPointInfo().host());
            }
            disconnectClient(fd, packetClientInfoReply->result());
            return -1;
        }
    }

    return 0;
}

void SlaveReactor::onClientsHeartbeatTimeout(const std::vector<int> &fds)
{
    for (const int fd: fds)
    {
        disconnectClient(fd);
    }
}