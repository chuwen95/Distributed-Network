//
// Created by ChuWen on 10/11/23.
//

#include "Initializer.h"

using namespace csm::initializer;

Initializer::Initializer(std::string configPath) : m_configPath(std::move(configPath))
{
}

int Initializer::init()
{
    m_nodeConfig = std::make_unique<tool::NodeConfig>(m_configPath);
    if (-1 == m_nodeConfig->init())
    {
        std::cerr << "Init config failed" << std::endl;
        return -1;
    }

    // 初始化log
    m_logInitializer = std::make_unique<AppLogInitializer>(m_nodeConfig.get());
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

    // 初始化P2PService
    m_p2pServiceInitializer = std::make_unique<P2PServiceInitializer>(m_nodeConfig.get());
    if (-1 == m_p2pServiceInitializer->init())
    {
        LOG->write(utilities::LogType::Log_Error, FILE_INFO, "TcpServiceInitializer init failed");
        return -1;
    }
    LOG->write(utilities::LogType::Log_Info, FILE_INFO, "TcpServiceInitializer init successfully");

    // 初始化Rpc
    m_rpcInitializer = std::make_unique<RpcInitializer>(m_nodeConfig.get(), m_p2pServiceInitializer->p2pService());
    if (-1 == m_rpcInitializer->init())
    {
        LOG->write(utilities::LogType::Log_Error, FILE_INFO, "RpcInitializer init failed");
        return -1;
    }
    LOG->write(utilities::LogType::Log_Info, FILE_INFO, "RpcInitializer init successfully");

    return 0;
}

int Initializer::start()
{
    if (-1 == m_p2pServiceInitializer->start())
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

    if (-1 == m_p2pServiceInitializer->stop())
    {
        LOG->write(utilities::LogType::Log_Error, FILE_INFO, "TcpServiceInitializer stop failed");
        return -1;
    }
    LOG->write(utilities::LogType::Log_Info, FILE_INFO, "TcpServiceInitializer stop successfully");

    if (-1 == m_logInitializer->stop())
    {
        std::cerr << "LogInitializer stop failed" << std::endl;
        return -1;
    }
    LOG->write(utilities::LogType::Log_Info, FILE_INFO, "LogInitializer stop successfully");

    return 0;
}

int Initializer::initP2PServiceModuleMessageHandler()
{
    m_p2pServiceInitializer->p2pService()->registerModulePacketHandler(protocol::ModuleID::raft,
        [this](const NodeId& nodeId, const std::vector<char>& data) -> int {
            return m_consensusInitializer->raft()->handleMessage(nodeId, data);
        });

    return 0;
}