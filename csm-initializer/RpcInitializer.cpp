//
// Created by ChuWen on 10/20/23.
//

#include "RpcInitializer.h"

#include "csm-rpc/tcp/TcpRpcFactory.h"
#include "csm-rpc/http/HttpRpcFactory.h"

using namespace csm::initializer;

RpcInitializer::RpcInitializer(tool::NodeConfig* nodeConfig, service::P2PService* p2pService)
{
    std::unique_ptr<rpc::RpcFactory> rpcFactory = std::make_unique<rpc::HttpRpcFactory>(nodeConfig, p2pService);
    m_httpRpcServer = rpcFactory->create();
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

csm::rpc::RpcServer* RpcInitializer::httpRpcServer()
{
    return m_httpRpcServer.get();
}

csm::rpc::RpcServer* RpcInitializer::tcpRpcServer()
{
    return m_tcpRpcServer.get();
}

