//
// Created by Chu on 2024/9/5.
//

#include "TcpRpcServer.h"

using namespace csm::rpc;

TcpRpcServer::TcpRpcServer(RpcConfig::Ptr rpcConfig) : RpcServer(std::move(rpcConfig))
{
    m_tcpService = service::P2PServiceFactory(m_rpcConfig->nodeConfig(), service::ServiceStartType::RpcServer).create();
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