//
// Created by ChuWen on 10/19/23.
//

#ifndef TCPNETWORK_RPCFACTORY_H
#define TCPNETWORK_RPCFACTORY_H

#include "csm-tool/NodeConfig.h"
#include "csm-service/P2PService.h"
#include "common/RpcServer.h"

namespace csm
{

    namespace rpc
    {

        enum class RpcServerType
        {
            HttpRpcServer,
            TcpRpcServer
        };

        class RpcFactory
        {
        public:
            using Ptr = std::shared_ptr<RpcFactory>;

            RpcFactory(tool::NodeConfig::Ptr nodeConfig, service::P2PService::Ptr tcpService);
            ~RpcFactory() = default;

        public:
            RpcServer::Ptr createRpc(const RpcServerType rpcServerType);

        private:
            tool::NodeConfig::Ptr m_nodeConfig;
            service::P2PService::Ptr m_tcpService;
        };

    } // rpc

}

#endif //TCPNETWORK_RPCFACTORY_H
