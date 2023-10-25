//
// Created by root on 10/19/23.
//

#include "RpcConfig.h"

namespace rpc
{

    RpcConfig::RpcConfig(tools::NodeConfig::Ptr nodeConfig, service::TcpService::Ptr tcpService, std::shared_ptr<httplib::Server> httpServer) :
            m_nodeConfig(std::move(nodeConfig)),
            m_tcpService(std::move(tcpService)),
            m_httpServer(std::move(httpServer))
    {}

    tools::NodeConfig::Ptr RpcConfig::nodeConfig()
    {
        return m_nodeConfig;
    }

    service::TcpService::Ptr RpcConfig::tcpService()
    {
        return m_tcpService;
    }

    std::shared_ptr<httplib::Server> RpcConfig::httpServer()
    {
        return m_httpServer;
    }

} // rpc