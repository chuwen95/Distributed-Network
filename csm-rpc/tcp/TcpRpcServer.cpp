//
// Created by Chu on 2024/9/5.
//

#include "TcpRpcServer.h"

using namespace csm::rpc;

TcpRpcServer::TcpRpcServer(std::unique_ptr<service::ServerService> service) : m_tcpService(std::move(service))
{
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