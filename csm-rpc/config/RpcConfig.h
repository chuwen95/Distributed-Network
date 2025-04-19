//
// Created by ChuWen on 10/19/23.
//

#ifndef TCPNETWORK_RPCCONFIG_H
#define TCPNETWORK_RPCCONFIG_H

#include "csm-service/P2PService.h"
#include "csm-tool/NodeConfig.h"
#include "httplib.h"

namespace csm
{

    namespace rpc
    {

        class RpcConfig
        {
        public:
            using Ptr = std::shared_ptr<RpcConfig>;

            RpcConfig(tool::NodeConfig::Ptr nodeConfig, service::P2PService::Ptr p2pService, std::shared_ptr<httplib::Server> httpServer);
            ~RpcConfig() = default;

        public:
            tool::NodeConfig::Ptr nodeConfig();
            service::P2PService::Ptr p2pService();

        private:
            service::P2PService::Ptr m_p2pService;
            tool::NodeConfig::Ptr m_nodeConfig;
        };

    } // rpc

}

#endif //TCPNETWORK_RPCCONFIG_H
