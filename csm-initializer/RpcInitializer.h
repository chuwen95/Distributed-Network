//
// Created by ChuWen on 10/20/23.
//

#ifndef TCPNETWORK_RPCINITIALIZER_H
#define TCPNETWORK_RPCINITIALIZER_H

#include "csm-tool/NodeConfig.h"
#include "csm-service/service/P2PService.h"
#include "csm-rpc/framework/RpcServer.h"

namespace csm
{

    namespace initializer
    {

        class RpcInitializer
        {
        public:
            explicit RpcInitializer(tool::NodeConfig* nodeConfig, service::P2PService* p2pService);
            ~RpcInitializer() = default;

        public:
            int init();

            int start();

            int stop();

            rpc::RpcServer* httpRpcServer();
            rpc::RpcServer* tcpRpcServer();

        private:
            std::unique_ptr<rpc::RpcServer> m_httpRpcServer;
            std::unique_ptr<rpc::RpcServer> m_tcpRpcServer;
        };

    } // initializer

}

#endif //TCPNETWORK_RPCINITIALIZER_H
