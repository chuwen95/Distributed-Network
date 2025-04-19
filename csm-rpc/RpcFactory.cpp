//
// Created by ChuWen on 10/20/23.
//

#include "RpcFactory.h"
#include "config/RpcConfig.h"
#include "http/HttpRpcServer.h"
#include "tcp/TcpRpcServer.h"

using namespace csm::rpc;

RpcFactory::RpcFactory(tool::NodeConfig::Ptr nodeConfig, service::P2PService::Ptr tcpService) :
        m_nodeConfig(std::move(nodeConfig)), m_tcpService(std::move(tcpService))
{}

RpcServer::Ptr RpcFactory::createRpc(const RpcServerType rpcServerType)
{
    std::shared_ptr<httplib::Server> httpServer = std::make_shared<httplib::Server>();
    RpcConfig::Ptr rpcConfig = std::make_shared<RpcConfig>(m_nodeConfig, m_tcpService, httpServer);

    if(RpcServerType::HttpRpcServer == rpcServerType)
    {
        return std::make_shared<HttpRpcServer>(rpcConfig);
    }
    else if(RpcServerType::TcpRpcServer == rpcServerType)
    {
        return std::make_shared<TcpRpcServer>(rpcConfig);
    }
    else
    {
        return nullptr;
    }
}
