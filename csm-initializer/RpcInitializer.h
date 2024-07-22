//
// Created by root on 10/20/23.
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

            RpcInitializer(tool::NodeConfig::Ptr nodeConfig, service::TcpService::Ptr tcpService);

            ~RpcInitializer() = default;

        public:
            int init();

            int uninit();

            int start();

            int stop();

            rpc::Rpc::Ptr rpc();

        private:
            rpc::Rpc::Ptr m_rpc;
        };

    } // initializer

}

#endif //TCPNETWORK_RPCINITIALIZER_H
