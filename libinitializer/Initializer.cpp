//
// Created by root on 10/11/23.
//

#include "Initializer.h"

#include "libcomponents/Logger.h"

namespace initializer
{

    int Initializer::initConfig(const std::string &configPath)
    {
        m_nodeConfig = std::make_shared<tool::NodeConfig>();
        return m_nodeConfig->init(configPath);
    }

    int Initializer::init()
    {
        // 设置Logger
        if(-1 == components::Singleton<components::Logger>::instance()->init(m_nodeConfig->enableFileLog(), m_nodeConfig->logPath()))
        {
            return -1;
        }
        components::Singleton<components::Logger>::instance()->setLogLevel(m_nodeConfig->logType());
        components::Singleton<components::Logger>::instance()->setConsoleOutput(m_nodeConfig->consoleOutput());

        // 初始化包处理器
        m_packetProcessInitializer = std::make_shared<PacketProcessorInitializer>();
        if(-1 == m_packetProcessInitializer->init())
        {
            components::Singleton<components::Logger>::instance()->write(components::LogType::Log_Error, FILE_INFO,
                                                                         "PacketProcessInitializer init failed");
            return -1;
        }
        components::Singleton<components::Logger>::instance()->write(components::LogType::Log_Info, FILE_INFO,
                                                                     "PacketProcessInitializer init successfully");

        // 初始化TcpService
        m_tcpServiceInitializer = std::make_shared<TcpServiceInitializer>(m_nodeConfig, m_packetProcessInitializer->packetProcessor());
        if(-1 == m_tcpServiceInitializer->init())
        {
            components::Singleton<components::Logger>::instance()->write(components::LogType::Log_Error, FILE_INFO,
                                                                         "TcpServiceInitializer init failed");
            return -1;
        }
        components::Singleton<components::Logger>::instance()->write(components::LogType::Log_Info, FILE_INFO,
                                                                     "TcpServiceInitializer init successfully");

        // 初始化Rpc
        m_rpcInitializer = std::make_shared<RpcInitializer>(m_nodeConfig, m_tcpServiceInitializer->tcpService());
        if(-1 == m_rpcInitializer->init())
        {
            components::Singleton<components::Logger>::instance()->write(components::LogType::Log_Error, FILE_INFO,
                                                                         "RpcInitializer init failed");
            return -1;
        }
        components::Singleton<components::Logger>::instance()->write(components::LogType::Log_Info, FILE_INFO,
                                                                     "RpcInitializer init successfully");

        return 0;
    }

    int Initializer::uninit()
    {
        if(-1 == m_rpcInitializer->uninit())
        {
            components::Singleton<components::Logger>::instance()->write(components::LogType::Log_Error, FILE_INFO,
                                                                         "RpcInitializer uninit failed");
            return -1;
        }
        components::Singleton<components::Logger>::instance()->write(components::LogType::Log_Info, FILE_INFO,
                                                                     "RpcInitializer uninit successfully");

        if(-1 == m_tcpServiceInitializer->uninit())
        {
            components::Singleton<components::Logger>::instance()->write(components::LogType::Log_Error, FILE_INFO,
                                                                         "TcpServiceInitializer uninit failed");
            return -1;
        }
        components::Singleton<components::Logger>::instance()->write(components::LogType::Log_Info, FILE_INFO,
                                                                     "TcpServiceInitializer uninit successfully");

        components::Singleton<components::Logger>::instance()->uninit();

        return 0;
    }

    int Initializer::start()
    {
        components::Singleton<components::Logger>::instance()->start();

        if(-1 == m_tcpServiceInitializer->start())
        {
            components::Singleton<components::Logger>::instance()->write(components::LogType::Log_Error, FILE_INFO,
                                                                         "TcpServiceInitializer start failed");
            return -1;
        }
        components::Singleton<components::Logger>::instance()->write(components::LogType::Log_Info, FILE_INFO,
                                                                     "TcpServiceInitializer start successfully");

        if(-1 == m_rpcInitializer->start())
        {
            components::Singleton<components::Logger>::instance()->write(components::LogType::Log_Error, FILE_INFO,
                                                                         "RpcInitializer start failed");
            return -1;
        }
        components::Singleton<components::Logger>::instance()->write(components::LogType::Log_Info, FILE_INFO,
                                                                     "RpcInitializer start successfully");

        return 0;
    }

    int Initializer::stop()
    {
        if(-1 == m_rpcInitializer->stop())
        {
            components::Singleton<components::Logger>::instance()->write(components::LogType::Log_Error, FILE_INFO,
                                                                         "RpcInitializer stop failed");
            return -1;
        }
        components::Singleton<components::Logger>::instance()->write(components::LogType::Log_Info, FILE_INFO,
                                                                     "RpcInitializer stop successfully");

        if(-1 == m_tcpServiceInitializer->stop())
        {
            components::Singleton<components::Logger>::instance()->write(components::LogType::Log_Error, FILE_INFO,
                                                                         "TcpServiceInitializer stop failed");
            return -1;
        }
        components::Singleton<components::Logger>::instance()->write(components::LogType::Log_Info, FILE_INFO,
                                                                     "TcpServiceInitializer stop successfully");

        components::Singleton<components::Logger>::instance()->stop();

        return 0;
    }

} // initializer