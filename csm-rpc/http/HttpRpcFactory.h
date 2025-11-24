//
// Created by ChuWen on 10/19/23.
//

#ifndef TCPNETWORK_HTTPRPCFACTORY_H
#define TCPNETWORK_HTTPRPCFACTORY_H

#include "csm-rpc/framework/RpcFactory.h"
#include "csm-service/service/P2PService.h"
#include "csm-rpc/framework/RpcServer.h"

namespace csm
{

    namespace rpc
    {

        class HttpRpcFactory : public RpcFactory
        {
        public:
            explicit HttpRpcFactory(tool::NodeConfig* nodeConfig, service::P2PService* tcpService);
            ~HttpRpcFactory() override = default;

        public:
            std::unique_ptr<RpcServer> create() override;

        private:
            tool::NodeConfig* m_nodeConfig{nullptr};
            service::P2PService* m_p2pService{nullptr};
        };

    } // rpc

}

#endif //TCPNETWORK_RPCFACTORY_H
