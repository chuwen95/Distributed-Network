//
// Created by Chu on 2024/9/5.
//

#include "TcpRpcServer.h"

using namespace csm::rpc;

TcpRpcServer::TcpRpcServer(RpcConfig::Ptr rpcConfig) : RpcServer(std::move(rpcConfig))
{
    service::TcpServiceFactory::Ptr tcpServiceFactory = std::make_shared<service::TcpServiceFactory>(m_rpcConfig->nodeConfig());
    m_tcpService = tcpServiceFactory->createTcpService(service::ServiceStartType::Server);
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