//
// Created by root on 10/11/23.
//

#include "Initializer.h"

namespace initializer
{

    int Initializer::initConfig(const std::string &configPath)
    {
        m_nodeConfig = std::make_shared<tools::NodeConfig>();
        return m_nodeConfig->init(configPath);
    }

    int Initializer::init()
    {
        // 初始化log
        m_logInitializer = std::make_shared<LogInitializer>(m_nodeConfig);
        if(-1 == m_logInitializer->init())
        {
            std::cerr << "LogInitializer init failed" << std::endl;
            return -1;
        }
        if(-1 == m_logInitializer->start())
        {
            std::cerr << "LogInitializer start failed" << std::endl;
            return -1;
        }

        // 初始化TcpService
        m_tcpServiceInitializer = std::make_shared<TcpServiceInitializer>(m_nodeConfig);
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

        if(-1 == m_logInitializer->stop())
        {
            std::cerr << "LogInitializer stop failed" << std::endl;
            return -1;
        }

        if(-1 == m_logInitializer->uninit())
        {
            std::cerr << "LogInitializer uninit failed" << std::endl;
            return -1;
        }

        return 0;
    }

    int Initializer::start()
    {

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

        return 0;
    }

} // initializer