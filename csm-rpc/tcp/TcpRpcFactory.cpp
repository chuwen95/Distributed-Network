//
// Created by ChuWen on 10/20/23.
//

#include "TcpRpcFactory.h"

#include "csm-service/factory/ServerServiceFactory.h"
#include "csm-rpc/tcp/TcpRpcServer.h"

using namespace csm::rpc;

TcpRpcFactory::TcpRpcFactory(tool::NodeConfig* nodeConfig) : m_nodeConfig(nodeConfig)
{
}

std::unique_ptr<RpcServer> TcpRpcFactory::create()
{
    service::ServerServiceFactory factory(m_nodeConfig);
    std::unique_ptr<service::ServerService> service = factory.
        create();
    return std::make_unique<TcpRpcServer>(std::move(service));
}