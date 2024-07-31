//
// Created by ChuWen on 10/19/23.
//

#ifndef TCPNETWORK_RPCFACTORY_H
#define TCPNETWORK_RPCFACTORY_H

#include "csm-tool/NodeConfig.h"
#include "csm-service/TcpService.h"
#include "Rpc.h"

namespace csm
{

    namespace rpc
    {

        class RpcFactory
        {
        public:
            using Ptr = std::shared_ptr<RpcFactory>;

            RpcFactory(tool::NodeConfig::Ptr nodeConfig, service::TcpService::Ptr tcpService);
            ~RpcFactory() = default;

        public:
            Rpc::Ptr createRpc();

        private:
            tool::NodeConfig::Ptr m_nodeConfig;
            service::TcpService::Ptr m_tcpService;
        };

    } // rpc

}

#endif //TCPNETWORK_RPCFACTORY_H
