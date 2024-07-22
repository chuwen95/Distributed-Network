//
// Created by root on 10/20/23.
//

#include "RpcFactory.h"
#include "config/RpcConfig.h"

using namespace csm::rpc;

RpcFactory::RpcFactory(tool::NodeConfig::Ptr nodeConfig, service::TcpService::Ptr tcpService) :
        m_nodeConfig(nodeConfig), m_tcpService(tcpService)
{}

Rpc::Ptr RpcFactory::createRpc()
{
    std::shared_ptr<httplib::Server> httpServer = std::make_shared<httplib::Server>();
    RpcConfig::Ptr rpcConfig = std::make_shared<RpcConfig>(m_nodeConfig, m_tcpService, httpServer);

    return std::make_shared<Rpc>(rpcConfig);
}
