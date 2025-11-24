//
// Created by ChuWen on 10/20/23.
//

#include "HttpRpcFactory.h"

#include "HttpRpcServer.h"

using namespace csm::rpc;

HttpRpcFactory::HttpRpcFactory(tool::NodeConfig* nodeConfig, service::P2PService* p2pService) :
        m_nodeConfig(nodeConfig), m_p2pService(p2pService) {}

std::unique_ptr<RpcServer> HttpRpcFactory::create()
{
    std::unique_ptr<httplib::Server> httpServer = std::make_unique<httplib::Server>();
    return std::make_unique<HttpRpcServer>(m_nodeConfig, std::move(httpServer), m_p2pService);
}
