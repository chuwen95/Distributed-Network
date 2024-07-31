//
// Created by ChuWen on 10/11/23.
//

#include "Initializer.h"

using namespace csm::initializer;

int Initializer::initConfig(const std::string &configPath)
{
    m_nodeConfig = std::make_shared<tool::NodeConfig>();
    return m_nodeConfig->init(configPath);
}

int Initializer::init()
{
    // 初始化log
    m_logInitializer = std::make_shared<AppLogInitializer>(m_nodeConfig);
    if (-1 == m_logInitializer->init())
    {
        std::cerr << "LogInitializer init failed" << std::endl;
        return -1;
    }
    if (-1 == m_logInitializer->start())
    {
        std::cerr << "LogInitializer start failed" << std::endl;
        return -1;
    }

    // 初始化TcpService
    m_tcpServiceInitializer = std::make_shared<TcpServiceInitializer>(m_nodeConfig);
    if (-1 == m_tcpServiceInitializer->init())
    {
        LOG->write(utilities::LogType::Log_Error, FILE_INFO, "TcpServiceInitializer init failed");
        return -1;
    }
    LOG->write(utilities::LogType::Log_Info, FILE_INFO, "TcpServiceInitializer init successfully");

    // 初始化Rpc
    m_rpcInitializer = std::make_shared<RpcInitializer>(m_nodeConfig, m_tcpServiceInitializer->tcpService());
    if (-1 == m_rpcInitializer->init())
    {
        LOG->write(utilities::LogType::Log_Error, FILE_INFO, "RpcInitializer init failed");
        return -1;
    }
    LOG->write(utilities::LogType::Log_Info, FILE_INFO, "RpcInitializer init successfully");

    return 0;
}

int Initializer::uninit()
{
    if (-1 == m_rpcInitializer->uninit())
    {
        LOG->write(utilities::LogType::Log_Error, FILE_INFO, "RpcInitializer uninit failed");
        return -1;
    }
    LOG->write(utilities::LogType::Log_Info, FILE_INFO, "RpcInitializer uninit successfully");

    if (-1 == m_tcpServiceInitializer->uninit())
    {
        LOG->write(utilities::LogType::Log_Error, FILE_INFO, "TcpServiceInitializer uninit failed");
        return -1;
    }
    LOG->write(utilities::LogType::Log_Info, FILE_INFO, "TcpServiceInitializer uninit successfully");

    if (-1 == m_logInitializer->stop())
    {
        std::cerr << "LogInitializer stop failed" << std::endl;
        return -1;
    }

    return 0;
}

int Initializer::start()
{
    if (-1 == m_tcpServiceInitializer->start())
    {
        LOG->write(utilities::LogType::Log_Error, FILE_INFO, "TcpServiceInitializer start failed");
        return -1;
    }
    LOG->write(utilities::LogType::Log_Info, FILE_INFO, "TcpServiceInitializer start successfully");

    if (-1 == m_rpcInitializer->start())
    {
        LOG->write(utilities::LogType::Log_Error, FILE_INFO, "RpcInitializer start failed");
        return -1;
    }
    LOG->write(utilities::LogType::Log_Info, FILE_INFO, "RpcInitializer start successfully");

    return 0;
}

int Initializer::stop()
{
    if (-1 == m_rpcInitializer->stop())
    {
        LOG->write(utilities::LogType::Log_Error, FILE_INFO, "RpcInitializer stop failed");
        return -1;
    }
    LOG->write(utilities::LogType::Log_Info, FILE_INFO, "RpcInitializer stop successfully");

    if (-1 == m_tcpServiceInitializer->stop())
    {
        LOG->write(utilities::LogType::Log_Error, FILE_INFO, "TcpServiceInitializer stop failed");
        return -1;
    }
    LOG->write(utilities::LogType::Log_Info, FILE_INFO, "TcpServiceInitializer stop successfully");

    return 0;
}