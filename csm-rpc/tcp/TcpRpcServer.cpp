//
// Created by Chu on 2024/9/5.
//

#include "TcpRpcServer.h"

using namespace csm::rpc;

TcpRpcServer::TcpRpcServer(RpcConfig::Ptr rpcConfig) : RpcServer(std::move(rpcConfig))
{
    service::P2PServiceFactory::Ptr tcpServiceFactory =
        std::make_shared<service::P2PServiceFactory>(m_rpcConfig->nodeConfig(), service::ServiceStartType::Server);
    m_tcpService = tcpServiceFactory->create();
}

int TcpRpcServer::init()
{
    return m_tcpService->init();
}

int TcpRpcServer::start()
{
    return m_tcpService->start();
}

int TcpRpcServer::stop()
{
    return m_tcpService->stop();
}