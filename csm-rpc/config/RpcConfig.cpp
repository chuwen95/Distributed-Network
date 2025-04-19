//
// Created by ChuWen on 10/19/23.
//

#include "RpcConfig.h"

using namespace csm::rpc;

RpcConfig::RpcConfig(tool::NodeConfig::Ptr nodeConfig, service::P2PService::Ptr p2pService, std::shared_ptr<httplib::Server> httpServer) :
        m_nodeConfig(std::move(nodeConfig)), m_p2pService(std::move(p2pService))
{}

csm::tool::NodeConfig::Ptr RpcConfig::nodeConfig()
{
    return m_nodeConfig;
}

csm::service::P2PService::Ptr RpcConfig::p2pService()
{
    return m_p2pService;
}