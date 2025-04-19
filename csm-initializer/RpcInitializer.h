//
// Created by ChuWen on 10/20/23.
//

#ifndef TCPNETWORK_RPCINITIALIZER_H
#define TCPNETWORK_RPCINITIALIZER_H

#include "csm-rpc/RpcFactory.h"

namespace csm
{

    namespace initializer
    {

        class RpcInitializer
        {
        public:
            using Ptr = std::shared_ptr<RpcInitializer>;

            RpcInitializer(tool::NodeConfig::Ptr nodeConfig, service::P2PService::Ptr tcpService);
            ~RpcInitializer() = default;

        public:
            int init();

            int start();

            int stop();

            rpc::RpcServer::Ptr httpRpcServer();
            rpc::RpcServer::Ptr tcpRpcServer();

        private:
            rpc::RpcServer::Ptr m_httpRpcServer;
            rpc::RpcServer::Ptr m_tcpRpcServer;
        };

    } // initializer

}

#endif //TCPNETWORK_RPCINITIALIZER_H
