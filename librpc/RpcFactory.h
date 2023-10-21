//
// Created by root on 10/19/23.
//

#ifndef TCPNETWORK_RPCFACTORY_H
#define TCPNETWORK_RPCFACTORY_H

#include "libtools/NodeConfig.h"
#include "libservice/TcpService.h"
#include "Rpc.h"

namespace rpc
{

    class RpcFactory
    {
    public:
        RpcFactory(tool::NodeConfig::Ptr nodeConfig, service::TcpService::Ptr tcpService);
        ~RpcFactory() = default;

    public:
        Rpc::Ptr createRpc();

    private:
        tool::NodeConfig::Ptr m_nodeConfig;
        service::TcpService::Ptr m_tcpService;
    };

} // rpc

#endif //TCPNETWORK_RPCFACTORY_H
