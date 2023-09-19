//
// Created by root on 9/5/23.
//

#include "SlaveReactor.h"

#include "libcomponents/Socket.h"
#include "libcomponents/Logger.h"
#include "libpacketprocess/PacketFactory.h"
#include "libpacketprocess/packet/PacketClientInfo.h"
#include "libpacketprocess/packet/PacketClientInfoReply.h"

namespace server
{

    constexpr std::size_t c_maxEvent{500};

    SlaveReactor::SlaveReactor(const int id) : m_id(id)
    {
    }

    SlaveReactor::~SlaveReactor()
    {
    }

    int SlaveReactor::init()
    {
        m_epfd = epoll_create1(0);
        if(-1 == m_epfd)
        {
            components::Singleton<components::Logger>::instance()->write(components::LogType::Log_Error, FILE_INFO,
                                                                         "create epoll fd failed, fd: ", m_epfd);
            return -1;
        }
        components::Singleton<components::Logger>::instance()->write(components::LogType::Log_Debug, FILE_INFO,
                                                                     "create epoll fd successfully, fd: ", m_epfd);

        m_clientAliveChecker.init();

        return 0;
    }

    int SlaveReactor::uninit()
    {
        m_clientAliveChecker.uninit();

        components::Socket::close(m_epfd);

        return 0;
    }

    int SlaveReactor::start()
    {
        const auto expression = [this]()
        {
            // 移除掉线的客户端
            std::vector<int> offlineClients;
            m_clientAliveChecker.getOfflineClient(offlineClients);
            for(const int fd : offlineClients)
            {
                components::Singleton<components::Logger>::instance()->write(components::LogType::Log_Info, FILE_INFO,
                                                                             "remove offline client: ", fd);
                onClientDisconnect(fd);
            }

            int timeout{0};
            if(true == m_infds.empty() && true == m_outfds.empty() && true == m_datafds.empty())
            {
                timeout = 10;
            }
            struct epoll_event ev[c_maxEvent];
            int nready = epoll_wait(m_epfd, ev, c_maxEvent, timeout);
            if(-1 == nready)
            {
                components::Singleton<components::Logger>::instance()->write(components::LogType::Log_Error, FILE_INFO,
                                                                             "epoll_wait failed, errno: " , errno, ", ", strerror(errno));
                return -1;
            }
            else if(0 == nready)
            {
            }

            for(int i = 0; i < nready; ++i)
            {
                int fd = ev[i].data.fd;

                // error
                if (ev[i].events & EPOLLERR ||         // 文件描述符发生错误
                    ev[i].events & EPOLLHUP)     // 文件描述符被挂断
                {
                    components::Singleton<components::Logger>::instance()->write(components::LogType::Log_Info, FILE_INFO,
                                                                                 "fd error, fd: ", fd);
                    onClientDisconnect(fd);
                    return -1;
                }

                if (ev[i].events & EPOLLIN)
                {
                    components::Singleton<components::Logger>::instance()->write(components::LogType::Log_Trace, FILE_INFO,
                                                                                 "EPOLLIN event, fd: ", fd);
                    {
                        std::unique_lock<std::mutex> ulock(x_clientSessions);
                        auto iter = m_fdSessions.find(fd);
                        if(m_fdSessions.end() == iter)
                        {
                            components::Singleton<components::Logger>::instance()->write(components::LogType::Log_Error, FILE_INFO,
                                                                                         "fd not found, fd: ", fd);
                            continue;
                        }
                        m_infds.insert(fd);
                    }
                }
                if(ev[i].events & EPOLLOUT)
                {
                    {
                        std::unique_lock<std::mutex> ulock(x_clientSessions);
                        auto iter = m_fdSessions.find(fd);
                        if(m_fdSessions.end() == iter)
                        {
                            components::Singleton<components::Logger>::instance()->write(components::LogType::Log_Error, FILE_INFO,
                                                                                         "fd not found, fd: ", fd);
                            continue;
                        }

                        if(0 == iter->second->writeBuffer()->dataLength())
                        {
                            continue;
                        }
                    }

                    std::unique_lock<std::mutex> ulock(x_outfds);

                    auto iter = m_outfds.find(fd);
                    if(m_outfds.end() != iter)
                    {
                        iter->second.second = true;
                    }
                }
            }

            // 处理fd读事件
            for(auto iter = m_infds.begin(); iter != m_infds.end();)
            {
                int fd = *iter;

                TcpSession::Ptr tcpSession;
                {
                    std::unique_lock<std::mutex> ulock(x_clientSessions);
                    auto sessionIter = m_fdSessions.find(fd);
                    if(m_fdSessions.end() == sessionIter)
                    {
                        components::Singleton<components::Logger>::instance()->write(components::LogType::Log_Error, FILE_INFO,
                                                                                     "fd not found, fd: ", fd);
                        iter = m_infds.erase(iter);
                        continue;
                    }
                    tcpSession = sessionIter->second;
                }

                components::RingBuffer::Ptr readBuffer = tcpSession->readBuffer();

                // 如果缓冲区满了，则无法在接收数据，继续外层for循环，但是由于边缘触发的原因，不能将fd从m_infds中移除
                char *data{nullptr};
                std::size_t length{0};
                int ret = readBuffer->getBufferAndLengthForWrite(data, length);
                if(-1 == ret)
                {
                    components::Singleton<components::Logger>::instance()->write(components::LogType::Log_Debug, FILE_INFO,
                                                                                 "getBufferAndLengthForWrite failed, fd: ", fd);
                    // 致命错误，缓冲区使用方法不正确
                    assert(-1 != ret);
                }
                components::Singleton<components::Logger>::instance()->write(components::LogType::Log_Debug, FILE_INFO,
                                                                             "getBufferAndLengthForWrite, length:", length, ", fd: ", fd);

                int readLen = recv(fd, data, length, 0);
                if (readLen < 0)
                {
                    // 没有数据再需要读取，从m_infds中移除
                    iter = m_infds.erase(iter);
                    break;
                }
                else if(0 == readLen)
                {
                    components::Singleton<components::Logger>::instance()->write(components::LogType::Log_Error, FILE_INFO,
                                                                                 "client may be offline, fd: ", fd);
                    iter = m_infds.erase(iter);
                    onClientDisconnect(fd, false);
                    continue;
                }
                // 增加已经使用的空间
                ret = readBuffer->increaseUsedSpace(readLen);
                if(-1 == ret)
                {
                    components::Singleton<components::Logger>::instance()->write(components::LogType::Log_Debug, FILE_INFO,
                                                                                 "increaseUsedSpace failed, space: ", readBuffer->space(), ", readLen: ", readLen, ", fd: ", fd);
                    // 致命错误，缓冲区使用方法不正确
                    assert(-1 != ret);
                }

                components::Singleton<components::Logger>::instance()->write(components::LogType::Log_Debug, FILE_INFO,
                                                                             "refresh last recv time, recv length: ", readLen, ", readBuffer dataLength: ", readBuffer->dataLength(), ", fd: ", fd);

                // 更新时间戳
                m_clientAliveChecker.refreshClientLastRecvTime(fd);

                // 接收到了数据，需要回调给业务层
                m_datafds.insert(fd);

                ++iter;
            }

            // 接收到了数据，需要回调给业务层
            for(auto iter = m_datafds.begin(); iter != m_datafds.end(); )
            {
                int fd = *iter;
                components::Singleton<components::Logger>::instance()->write(components::LogType::Log_Trace, FILE_INFO,
                                                                             "try to decode packet, fd: ", fd);

                TcpSession::Ptr tcpSession{nullptr};
                {
                    std::unique_lock<std::mutex> ulock(x_clientSessions);
                    auto sessionIter = m_fdSessions.find(fd);
                    if(m_fdSessions.end() == sessionIter)
                    {
                        components::Singleton<components::Logger>::instance()->write(components::LogType::Log_Error, FILE_INFO,
                                                                                     "fd not found, fd: ", fd);
                        iter = m_datafds.erase(iter);
                        continue;
                    }
                    tcpSession = sessionIter->second;
                }

                components::RingBuffer::Ptr readBuffer = tcpSession->readBuffer();
                if(0 == readBuffer->dataLength())
                {
                    components::Singleton<components::Logger>::instance()->write(components::LogType::Log_Error, FILE_INFO,
                                                                                 "fd: ", fd);
                    assert(0 != readBuffer->dataLength());
                }

#if 0
                components::CellTimestamp timestamp;
                timestamp.update();
#endif
                std::size_t epochPacketSum{100};
                std::size_t packetNum{0};
                for(packetNum = 0; packetNum < epochPacketSum; ++packetNum)
                {
                    components::Singleton<components::Logger>::instance()->write(components::LogType::Log_Debug, FILE_INFO,
                                                                                 "readBuffer length: ", readBuffer->dataLength(), ", startOffset: ", readBuffer->startOffset(), ", endOffset: ", readBuffer->endOffset(), ", fd: ", fd);

                    packetprocess::PacketType packetType;
                    std::shared_ptr<std::vector<char>> packetPayload = std::make_shared<std::vector<char>>();
                    int ret = getPacket(fd, readBuffer, packetType, packetPayload);
                    if(0 != ret)
                    {
                        // 缓冲区中的数据已经不够一个包头的长度
                        components::Singleton<components::Logger>::instance()->write(components::LogType::Log_Debug, FILE_INFO,
                                                                                     "pop packet finish", ", fd: ", fd);
                        break;
                    }

                    if(packetprocess::PacketType::PT_None == packetType)
                    {
                        components::Singleton<components::Logger>::instance()->write(components::LogType::Log_Error, FILE_INFO,
                                                                                     "decode packet failed, fd: ", fd);
                    }
                    else if(packetprocess::PacketType::PT_ClientInfo == packetType)
                    {
                        // 表明身份的包，即刻处理，不必放到线程池中
                        components::Singleton<components::Logger>::instance()->write(components::LogType::Log_Info, FILE_INFO,
                                                                                     "recv ClientInfo packet, fd: ", fd);
                        packetprocess::PacketBase::Ptr packet = packetprocess::PacketFactory().createPacket(packetType, packetPayload);
                        if(-1 == processClientInfoPacket(fd, tcpSession, packet))
                        {
                            components::Singleton<components::Logger>::instance()->write(components::LogType::Log_Error, FILE_INFO,
                                                                                         "process ClientInfo packet failed, fd: ", fd);
                        }
                    }
                    else if(packetprocess::PacketType::PT_HeartBeat == packetType)
                    {
                        // 心跳包，不必放到线程池中处理
                        components::Singleton<components::Logger>::instance()->write(components::LogType::Log_Info, FILE_INFO,
                                                                                     "receive heartbeat packet, fd: ", fd);
                        m_clientAliveChecker.refreshClientLastRecvTime(fd);
                    }
                    else
                    {
                        components::Singleton<components::Logger>::instance()->write(components::LogType::Log_Debug, FILE_INFO,
                                                                                     "callback packet type and packet payload, fd: ", fd, ", packet type: ", static_cast<int>(packetType));

                        std::weak_ptr<SlaveReactor> weakSlaveReactor = shared_from_this();
                        m_recvHandler(fd, packetType, packetPayload,[weakSlaveReactor](const int fd, const std::vector<char> &data) -> int {
                            auto slaveReactor = weakSlaveReactor.lock();
                            if (nullptr != slaveReactor)
                            {
                                return slaveReactor->sendData(fd, data.data(), data.size());
                            }
                            return 0;
                        });
                    }
                }
                // 1. 如果刚好取出epochPacketNum后缓冲区为空了，packetNum会等于epochPacketSum，需要判断下缓冲区是否还有数据
                // 2. 如果packetNum < epochPacketSum，那肯定是缓冲区数据不够一个包了，直接从m_datafds中移除
                if(0 == readBuffer->dataLength() || packetNum < epochPacketSum)
                {
                    iter = m_datafds.erase(iter);
                }
                else
                {
                    ++iter;
                }
                components::Singleton<components::Logger>::instance()->write(components::LogType::Log_Debug, FILE_INFO,
                                                                             "decode packet finish, fd: ", fd, ", packet num: ", packetNum);
#if 0
                components::Singleton<components::Logger>::instance()->write(components::LogType::Log_Debug, FILE_INFO,
                                                                             "elapsed time: ", timestamp.getElapsedTimeInMilliSec());
#endif
            }

            // 处理fd写
            std::unordered_map<int, std::pair<bool, bool>> outfds;
            {
                std::unique_lock<std::mutex> ulock(x_outfds);
                outfds = m_outfds;
            }
            for(auto iter = outfds.begin(); iter != outfds.end();)
            {
                int fd = iter->first;

                TcpSession::Ptr tcpSession{nullptr};
                {
                    // 这里之所以还要判断一遍是处理这种情况：
                    // 如果在sendData中写入到缓冲区后加入到m_outfds前客户端离线m_fdSessions中TCPSession被移除
                    std::unique_lock<std::mutex> ulock(x_clientSessions);
                    auto sessionIter = m_fdSessions.find(fd);
                    if(m_fdSessions.end() == sessionIter)
                    {
                        components::Singleton<components::Logger>::instance()->write(components::LogType::Log_Error, FILE_INFO,
                                                                                     "fd not found, fd: ", fd);
                        iter = outfds.erase(iter);
                        continue;
                    }
                    tcpSession = sessionIter->second;
                }

                if(true == iter->second.first && false == iter->second.second)
                {
                    // 如果fd正在等待EPOLLOUT事件且EPOLLOUT事件没来
                    ++iter;
                    continue;
                }

                components::RingBuffer::Ptr writeBuffer = tcpSession->writeBuffer();
                if(0 == writeBuffer->dataLength())
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
                        components::Singleton<components::Logger>::instance()->write(components::LogType::Log_Debug,FILE_INFO,
                                                                                     "send data successfully, fd: ", fd, ", sendlen: ", sendLen);
                        if(0 == writeBuffer->dataLength())
                        {
                            // 如果发送缓冲区中数据都没有了，直接从outfds中移除fd
                            iter = outfds.erase(iter);
                            continue;
                        }
                        else
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
                            components::Singleton<components::Logger>::instance()->write(components::LogType::Log_Debug, FILE_INFO,
                                                                                         "send data error, fd: ", fd, ", errno: ", errno, ", ", strerror(errno));
                        }
                        else if(ECONNRESET == errno || EPIPE == errno)
                        {
                            // 表明对端断开连接
                            components::Singleton<components::Logger>::instance()->write(components::LogType::Log_Error, FILE_INFO,
                                                                                         "client maybe offline, fd: ", fd, ", errno: ", errno, ", ", strerror(errno));

                            iter = outfds.erase(iter);
                            onClientDisconnect(fd);

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
        std::string threadName = "slav_reac_" + std::to_string(m_id);
        m_thread.init(expression, 0, threadName.c_str());
        m_thread.start();

        // 启动客户端在线监测
        m_clientAliveChecker.start();

        return 0;
    }

    int SlaveReactor::stop()
    {
        // 停止客户端在线监测
        m_clientAliveChecker.stop();

        m_thread.stop();
        m_thread.uninit();

        return 0;
    }

    int SlaveReactor::addClient(TcpSession::Ptr tcpSession)
    {
        components::Singleton<components::Logger>::instance()->write(components::LogType::Log_Info, FILE_INFO, "SlaveReactor ",
                                                                     m_id, " try to add client ", tcpSession->fd(), " to epoll");

        int fd = tcpSession->fd();

        {
            std::unique_lock<std::mutex> ulock(x_clientSessions);
            m_fdSessions.emplace(fd, tcpSession);
        }

        m_clientAliveChecker.addClient(fd);

        components::Singleton<components::Logger>::instance()->write(components::LogType::Log_Info, FILE_INFO, "SlaveReactor ", m_id,
                                                                     " add client ", fd, " to ClientAliveChecker successfully");

        struct epoll_event ev;
        memset(&ev, 0, sizeof(struct epoll_event));
        ev.events = EPOLLIN | EPOLLOUT | EPOLLET;
        ev.data.fd = fd;
        int ret = epoll_ctl(m_epfd, EPOLL_CTL_ADD, fd, &ev);
        if(-1 == ret)
        {
            components::Singleton<components::Logger>::instance()->write(components::LogType::Log_Error, FILE_INFO, "epoll_ctl add failed, fd: ",
                                                                         fd, ", errno: " , errno, ", ", strerror(errno));

            {
                std::unique_lock<std::mutex> ulock(x_clientSessions);
                m_fdSessions.erase(fd);
            }

            m_clientAliveChecker.removeClient(fd);

            return -1;
        }

        components::Singleton<components::Logger>::instance()->write(components::LogType::Log_Info, FILE_INFO, "SlaveReactor ", m_id,
                                                                     " add client ", tcpSession->fd(), " to epoll successfully, events: EPOLLIN and EPOLLET", ", m_epfd: ", m_epfd);

        return 0;
    }

    std::size_t SlaveReactor::clientSize()
    {
        std::unique_lock<std::mutex> ulock(x_clientSessions);
        return m_fdSessions.size();
    }

    void SlaveReactor::registerRecvHandler(std::function<void(const int, const packetprocess::PacketType,
                                                              std::shared_ptr<std::vector<char>>&, std::function<int(const int, const std::vector<char>&)>)> recvHandler)
    {
        m_recvHandler = std::move(recvHandler);
    }

    void SlaveReactor::registerDisconnectHandler(std::function<void(const int, const std::string&)> disconnectHandler)
    {
        m_disconnectHandler = disconnectHandler;
    }

    int SlaveReactor::sendData(const int fd, const char *data, const std::size_t size)
    {
        components::Singleton<components::Logger>::instance()->write(components::LogType::Log_Debug, FILE_INFO,
                                                                     "m_id: ", m_id, ", fd: ", fd);
        //获取对应的writeBuffer
        TcpSession::Ptr tcpSession;
        {
            std::unique_lock<std::mutex> ulock(x_clientSessions);

            auto iter = m_fdSessions.find(fd);
            if(m_fdSessions.end() == iter)
            {
                components::Singleton<components::Logger>::instance()->write(components::LogType::Log_Error, FILE_INFO, "id not found, id: ", fd);
                return -2;
            }
            tcpSession = iter->second;
        }

        {
            std::unique_lock<std::mutex> ulock(x_writeBuffer);
            components::RingBuffer::Ptr writeBuffer = tcpSession->writeBuffer();

            // 拷贝数据到缓冲区
            if (-1 == writeBuffer->writeData(data, size))
            {
                // 缓冲区放不下，返回错误给业务层
                components::Singleton<components::Logger>::instance()->write(components::LogType::Log_Error, FILE_INFO,
                                                                             "buffer not enough, fd: ", fd);
                return -1;
            }
            writeBuffer->increaseUsedSpace(size);
            components::Singleton<components::Logger>::instance()->write(components::LogType::Log_Debug,
                                                                         FILE_INFO, "write data to buffer successfully, size: ", size, ", fd: ", fd);
        }

        // fd有数据发送
        std::unique_lock<std::mutex> ulock(x_outfds);
        if(m_outfds.end() == m_outfds.find(tcpSession->fd()))
        {
            m_outfds.emplace(tcpSession->fd(), std::make_pair(false, false));
        }

        return 0;
    }

    std::uint32_t SlaveReactor::getClientOnlineTimestamp(const int fd)
    {
        std::unique_lock<std::mutex> ulock(x_clientSessions);

        auto iter = m_fdSessions.find(fd);
        if(m_fdSessions.end() == iter)
        {
            return 0;
        }
        return iter->second->getClientOnlineTimestamp();
    }

    void SlaveReactor::onClientDisconnect(const int fd, const bool dealInfds)
    {
        {
            std::unique_lock<std::mutex> ulock(x_clientSessions);
            auto iter = m_fdSessions.find(fd);
            if(m_fdSessions.end() == iter)
            {
                components::Singleton<components::Logger>::instance()->write(components::LogType::Log_Info, FILE_INFO,
                                                                             "client not exist, fd: ", fd);
                return;
            }
        }

        // 将客户端从在线监测中移除
        m_clientAliveChecker.removeClient(fd);

        epoll_ctl(m_epfd, EPOLL_CTL_DEL, fd, nullptr);

        if(true == dealInfds)
        {
            auto inIter = m_infds.find(fd);
            if(m_infds.end() != inIter)
            {
                m_infds.erase(inIter);
            }
        }

        auto dataIter = m_datafds.find(fd);
        if(m_datafds.end() != dataIter)
        {
            m_datafds.erase(dataIter);
        }

        {
            std::unique_lock<std::mutex> ulock(x_outfds);
            auto outIter = m_outfds.find(fd);
            if(m_outfds.end() != outIter)
            {
                m_outfds.erase(outIter);
            }
        }

        close(fd);

        {
            std::unique_lock<std::mutex> ulock(x_clientSessions);

            auto iter = m_fdSessions.find(fd);
            if(nullptr != m_disconnectHandler)
            {
                std::string id = iter->second->getClientId();
                m_disconnectHandler(fd, id);
            }
            m_fdSessions.erase(fd);
        }

        components::Singleton<components::Logger>::instance()->write(components::LogType::Log_Info, FILE_INFO,
                                                                     "client disconnect finish, fd: ", fd);
    }

    int SlaveReactor::getPacket(const int fd, components::RingBuffer::Ptr& readBuffer, packetprocess::PacketType &packetType, std::shared_ptr<std::vector<char>>& data)
    {
        packetprocess::PacketHeader packetHeader;
        const std::size_t headerLength = packetHeader.headerLength();

        components::Singleton<components::Logger>::instance()->write(components::LogType::Log_Debug, FILE_INFO,
                                                                     "readBuffer->length(): ", readBuffer->dataLength());
        if(readBuffer->dataLength() < headerLength)
        {
            components::Singleton<components::Logger>::instance()->write(components::LogType::Log_Info, FILE_INFO,
                                                                         "buffer data length less than header length, fd: ", fd);
            return -1;
        }

        char *buffer{nullptr};
        std::vector<char> bufferForBackspaceLessThanHeaderLength;

        int ret = readBuffer->getBufferForRead(headerLength, buffer);
        if(-2 == ret)
        {
            bufferForBackspaceLessThanHeaderLength.resize(headerLength);
            readBuffer->readData(headerLength, 0, bufferForBackspaceLessThanHeaderLength);
            buffer = bufferForBackspaceLessThanHeaderLength.data();
        }
        else if(-1 == ret)
        {
            return -1;
        }

        packetHeader.decode(buffer, headerLength);
        assert(true == packetHeader.isMagicMatch());
        packetType = packetHeader.type();

        components::Singleton<components::Logger>::instance()->write(components::LogType::Log_Debug, FILE_INFO,
                                                                     "decode packet header successfully, packet type: ", static_cast<int>(packetType), ", payload length: ", packetHeader.payloadLength(), ", fd: ", fd);

        // 尝试复制包数据
        std::size_t payloadLength = packetHeader.payloadLength();
        if(readBuffer->dataLength() - headerLength < payloadLength)
        {
            components::Singleton<components::Logger>::instance()->write(components::LogType::Log_Trace, FILE_INFO,
                                                                         "payload length not enough, packet type: ", static_cast<int>(packetType)," fd: ", fd);
            return -1;
        }

        data->resize(payloadLength);
        if(-1 == (ret= readBuffer->readData(payloadLength, headerLength, *data)))
        {
            components::Singleton<components::Logger>::instance()->write(components::LogType::Log_Trace, FILE_INFO,
                                                                         "payload length not enough, packet type: ", static_cast<int>(packetType)," fd: ", fd);
            assert(-1 != ret);
        }

        readBuffer->decreaseUsedSpace(headerLength + payloadLength);

        components::Singleton<components::Logger>::instance()->write(components::LogType::Log_Debug, FILE_INFO,
                                                                     "readBuffer dataLength: ", readBuffer->dataLength()," fd: ", fd);

        return 0;
    }

    int SlaveReactor::processClientInfoPacket(const int fd, TcpSession::Ptr tcpSession, packetprocess::PacketBase::Ptr packet)
    {
        if(nullptr == packet)
        {
            components::Singleton<components::Logger>::instance()->write(components::LogType::Log_Error, FILE_INFO,
                                                                         "decode ClientInfo packet failed", ", fd: ", fd);
            return -1;
        }

        packetprocess::PacketClientInfo::Ptr packetClientInfo = std::dynamic_pointer_cast<packetprocess::PacketClientInfo>(packet);
        std::string id = packetClientInfo->getId();
        tcpSession->setClientId(id);
        tcpSession->setClientOnlineTimestamp(components::CellTimestamp::getCurrentTimestamp());

        components::Singleton<components::Logger>::instance()->write(components::LogType::Log_Info, FILE_INFO,
                                                                     "decode ClientInfo packet successfully, fd: ", fd, ", id: ", fd);

        // 构造回应包
        packetprocess::PacketClientInfoReply reply;
        reply.setResult(0);

        packetprocess::PacketHeader packetHeader;
        packetHeader.setType(packetprocess::PacketType::PT_ClientInfoReply);
        packetHeader.setPayloadLength(reply.packetLength());

        int headerLength = packetHeader.headerLength();
        int payloadLength = reply.packetLength();

        std::vector<char> buffer;
        buffer.resize(headerLength + reply.packetLength());

        packetHeader.encode(buffer.data(), headerLength);
        reply.encode(buffer.data() + headerLength, payloadLength);

        if(-1 == sendData(fd, buffer.data(), buffer.size()))
        {
            components::Singleton<components::Logger>::instance()->write(components::LogType::Log_Error, FILE_INFO,
                                                                         "send ClientInfoReply packet failed", ", fd: ", fd, ", id: ", id);
        }

        components::Singleton<components::Logger>::instance()->write(components::LogType::Log_Info, FILE_INFO,
                                                                     "send ClientInfoReply packet successfully", ", fd: ", fd, ", id: ", id);

        return 0;
    }

}