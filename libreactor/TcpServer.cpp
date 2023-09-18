//
// Created by root on 9/5/23.
//

#include "TcpServer.h"
#include "libcomponents/Socket.h"
#include "libcomponents/Logger.h"
#include "libpacketprocess/PacketFactory.h"

namespace server
{

    constexpr std::size_t c_conQueueSize{500};

    int TcpServer::init(const std::string_view ip, const unsigned short port, const std::string_view logPath)
    {
        components::Singleton<components::Logger>::instance()->init(logPath, 4*1024*1024);
        components::Singleton<components::Logger>::instance()->setConsoleOutput(true);

        m_ip = ip;
        m_port = port;

        // 创建监听套接字
        m_fd = components::Socket::create();
        if(-1 == m_fd)
        {
            components::Singleton<components::Logger>::instance()->write(components::LogType::Log_Error, FILE_INFO, "create socket failed, errno: ", errno, ", ",
                                                                         strerror(errno));
            components::Socket::close(m_fd);
            return -1;
        }
        components::Singleton<components::Logger>::instance()->write(components::LogType::Log_Info, FILE_INFO, "create socket successfully, fd: ", m_fd);

#if 0   // 对于没有边缘触发的select来说，设置监听套接字非阻塞好像用处不大
        // 设置为非阻塞模式
        if(-1 == components::Socket::setNonBlock(m_fd))
        {
            return -1;
        }
#endif

        // 绑定监听地址端口
        if(-1 == components::Socket::bind(m_fd, ip, port))
        {
            components::Singleton<components::Logger>::instance()->write(components::LogType::Log_Error, FILE_INFO, "bind socket failed, errno: ", errno, ", ",
                                                                         strerror(errno));
            components::Socket::close(m_fd);
            return -1;
        }
        components::Singleton<components::Logger>::instance()->write(components::LogType::Log_Info, FILE_INFO, "bind socket successfully");

        // 设置套接字为被动监听及连接队列长度
        if(-1 == components::Socket::listen(m_fd, c_conQueueSize))
        {
            components::Singleton<components::Logger>::instance()->write(components::LogType::Log_Error, FILE_INFO, "listen socket failed, errno: ", errno, ", ",
                                                                         strerror(errno));
            components::Socket::close(m_fd);
            return -1;
        }
        components::Singleton<components::Logger>::instance()->write(components::LogType::Log_Info, FILE_INFO, "listen socket successfully");

        if(-1 == m_slaveReactorManager.init())
        {
            components::Singleton<components::Logger>::instance()->write(components::LogType::Log_Error, FILE_INFO, "slave reactor manager init failed");
            return -1;
        }
        components::Singleton<components::Logger>::instance()->write(components::LogType::Log_Info, FILE_INFO, "slave reactor manager init successfully");

        if(-1 == m_acceptor.init(m_fd))
        {
            components::Singleton<components::Logger>::instance()->write(components::LogType::Log_Error, FILE_INFO, "acceptor init failed");
            return -1;
        }
        components::Singleton<components::Logger>::instance()->write(components::LogType::Log_Info, FILE_INFO, "acceptor init successfully");

        if(-1 == m_selectListenner.init(m_fd))
        {
            components::Singleton<components::Logger>::instance()->write(components::LogType::Log_Error, FILE_INFO, "select listenner init failed");
            return -1;
        }
        components::Singleton<components::Logger>::instance()->write(components::LogType::Log_Info, FILE_INFO, "select listenner init successfully");

        // 注册连接时间回调
        m_selectListenner.registerConnectHandler([&](){ m_acceptor.onConnect(); });

        // 完成一个客户端的创建，将客户端分配到从Reactor进行recv/send处理
        m_acceptor.setNewClientCallback([this](const int fd, TcpSession* tcpSession){
            m_slaveReactorManager.addTcpSession(tcpSession);
        });

        m_packetProcessThreadPoll.init(8, "packet_proc");
        m_slaveReactorManager.registerRecvHandler([this](const int fd, packetprocess::PacketType packetType,
                                                         std::shared_ptr<std::vector<char>>& payloadData, const std::function<int(const int, const std::vector<char>&)>& writeHandler){
            const auto expression = [fd, packetType, payloadData, writeHandler, this]()
            {
                packetprocess::PacketFactory packetFactory;
                packetprocess::PacketBase::Ptr reqPacket = packetFactory.createPacket(packetType, payloadData);
                packetprocess::PacketReplyBase::Ptr replyPacket = packetFactory.createReplyPacket(packetType);
                if(nullptr != m_packetHander)
                {
                    if(-1 == m_packetHander(packetType, reqPacket, replyPacket))
                    {
                        components::Singleton<components::Logger>::instance()->write(components::LogType::Log_Info, FILE_INFO, "process packet failed");
                        return -1;
                    }
                }

                std::size_t payloadLength = replyPacket->packetLength();
                std::vector<char> buffer;

                packetprocess::PacketHeader packetHeader;
                packetHeader.setType(packetprocess::PacketType::PT_RawStringReply);
                packetHeader.setPayloadLength(payloadLength);

                std::size_t headerLength = packetHeader.headerLength();
                std::size_t sumLength = headerLength + payloadLength;

                // 编码包
                buffer.resize(sumLength);
                packetHeader.encode(buffer.data(), headerLength);
                replyPacket->encode(buffer.data() + headerLength, payloadLength);

                if(nullptr != writeHandler)
                {
                    // 将回应包写入发送缓冲区
                    components::Singleton<components::Logger>::instance()->write(components::LogType::Log_Trace, FILE_INFO,
                                                                                 "try write reply packet to buffer, write size: ", sumLength, ", fd: ", fd);
                    int ret{0};
                    while(0 != (ret = writeHandler(fd, buffer)))
                    {
                        if(-2 == ret)
                        {
                            components::Singleton<components::Logger>::instance()->write(components::LogType::Log_Trace, FILE_INFO,
                                                                                         " client offline, fd: ", fd);
                            break;
                        }
                        else if(-1 == ret)
                        {
                            std::this_thread::sleep_for(std::chrono::milliseconds(10));
                        }
                    }
                    components::Singleton<components::Logger>::instance()->write(components::LogType::Log_Trace, FILE_INFO,
                                                                                 " write reply packet to buffer successfully, write size: ", sumLength, ", fd: ", fd);
                }

                return 0;
            };
            m_packetProcessThreadPoll.push(expression);
        });

        return 0;
    }

    int TcpServer::uninit()
    {
        if(-1 == m_slaveReactorManager.uninit())
        {
            return -1;
        }

        if(-1 == m_acceptor.uninit())
        {
            return -1;
        }

        if(-1 == m_selectListenner.uninit())
        {
            return -1;
        }

        if(-1 == m_packetProcessThreadPoll.uninit())
        {
            return -1;
        }

        // 关闭套接字
        if(-1 == components::Socket::close(m_fd))
        {
            return -1;
        }

        components::Singleton<components::Logger>::instance()->uninit();

        return 0;
    }

    int TcpServer::start()
    {
        components::Singleton<components::Logger>::instance()->start();

        m_packetProcessThreadPoll.start();
        m_slaveReactorManager.start();
        m_acceptor.start();
        m_selectListenner.start();

        return 0;
    }

    int TcpServer::stop()
    {
        m_selectListenner.stop();
        m_acceptor.stop();
        m_slaveReactorManager.stop();
        m_packetProcessThreadPoll.stop();

        components::Singleton<components::Logger>::instance()->stop();

        return 0;
    }

    void TcpServer::setLogLevel(const int logLevel)
    {
        components::Singleton<components::Logger>::instance()->setLogLevel(static_cast<components::LogType>(logLevel));
    }

    void TcpServer::registerPacketHandler(std::function<int(const packetprocess::PacketType, packetprocess::PacketBase::Ptr,
                               packetprocess::PacketReplyBase::Ptr)> packetHander)
    {
        m_packetHander = std::move(packetHander);
    }

    void TcpServer::registerDisconnectHandler(std::function<void(const int, const std::string&)> disconnectHandler)
    {
        m_slaveReactorManager.registerDisconnectHandler(disconnectHandler);
    }



}