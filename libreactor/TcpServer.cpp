//
// Created by root on 9/5/23.
//

#include "TcpServer.h"
#include "libcomponents/Socket.h"
#include "libcomponents/Logger.h"
#include "libpacketprocess/packet/PacketBase.h"


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

        m_packetProcessor.init();
        m_slaveReactorManager.registerRecvHandler([this](const int fd, packetprocess::PacketType packetType,
                                                         std::shared_ptr<std::vector<char>>& payloadData, const std::function<int(const int, const std::vector<char>&)>& writeHandler){
            m_packetProcessor.processData(fd, packetType, payloadData, writeHandler);
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

        if(-1 == m_packetProcessor.uninit())
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

        m_packetProcessor.start();
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
        m_packetProcessor.stop();

        components::Singleton<components::Logger>::instance()->stop();

        return 0;
    }

    void TcpServer::setLogLevel(const int logLevel)
    {
        components::Singleton<components::Logger>::instance()->setLogLevel(static_cast<components::LogType>(logLevel));
    }

    void TcpServer::registerDisconnectHandler(std::function<void(const int)> disconnectHandler)
    {
        m_slaveReactorManager.registerDisconnectHandler(disconnectHandler);
    }

    int TcpServer::sendData(const int id, const char* data, const std::size_t size)
    {
        return m_slaveReactorManager.sendData(id, data,size);
    }

}