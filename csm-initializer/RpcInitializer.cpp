//
// Created by ChuWen on 10/20/23.
//

#include "RpcInitializer.h"
#include "csm-rpc/RpcFactory.h"

using namespace csm::initializer;

RpcInitializer::RpcInitializer(tool::NodeConfig::Ptr nodeConfig, service::P2PService::Ptr p2pService)
{
    rpc::RpcFactory rpcFactory(nodeConfig, p2pService);
    m_httpRpcServer = rpcFactory.createRpc(rpc::RpcServerType::HttpRpcServer);
#if 0
    m_tcpRpcServer = rpcFactory.createRpc(rpc::RpcServerType::TcpRpcServer);
#endif
}

int RpcInitializer::init()
{
    if(-1 == m_httpRpcServer->init())
    {
        return -1;
    }
#if 0
    if(-1 == m_tcpRpcServer->init())
    {
        return -1;
    }
#endif

    return 0;
}

int RpcInitializer::start()
{
    if(-1 == m_httpRpcServer->start())
    {
        return -1;
    }
#if 0
    if(-1 == m_tcpRpcServer->start())
    {
        return -1;
    }
#endif

    return 0;
}

int RpcInitializer::stop()
{
    if(-1 == m_httpRpcServer->stop())
    {
        return -1;
    }
#if 0
    if(-1 == m_tcpRpcServer->stop())
    {
        return -1;
    }
#endif

    return 0;
}

csm::rpc::RpcServer::Ptr RpcInitializer::httpRpcServer()
{
    return m_httpRpcServer;
}

csm::rpc::RpcServer::Ptr RpcInitializer::tcpRpcServer()
{
    return m_tcpRpcServer;
}

