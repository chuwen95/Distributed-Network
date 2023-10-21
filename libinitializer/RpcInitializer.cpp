//
// Created by root on 10/20/23.
//

#include "RpcInitializer.h"
#include "librpc/RpcFactory.h"

namespace initializer
{

    RpcInitializer::RpcInitializer(tool::NodeConfig::Ptr nodeConfig, service::TcpService::Ptr tcpService)
    {
        rpc::RpcFactory rpcFactory(nodeConfig, tcpService);
        m_rpc = rpcFactory.createRpc();
    }

    int RpcInitializer::init()
    {
        return m_rpc->init();
    }

    int RpcInitializer::uninit()
    {
        return m_rpc->uninit();
    }

    int RpcInitializer::start()
    {
        return m_rpc->start();
    }

    int RpcInitializer::stop()
    {
        return m_rpc->stop();
    }

    rpc::Rpc::Ptr RpcInitializer::rpc()
    {
        return m_rpc;
    }

} // initializer