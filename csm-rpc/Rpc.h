//
// Created by ChuWen on 10/19/23.
//

#ifndef TCPNETWORK_RPC_H
#define TCPNETWORK_RPC_H

#include "csm-common/Common.h"
#include "csm-rpc/config/RpcConfig.h"

namespace csm
{

    namespace rpc
    {

        class Rpc
        {
        public:
            using Ptr = std::shared_ptr<Rpc>;

            Rpc(RpcConfig::Ptr rpcConfig);
            ~Rpc() = default;

        public:
            int init();

            int start();

            int stop();

        private:
            RpcConfig::Ptr m_rpcConfig;
        };

    } // rpc

}

#endif //TCPNETWORK_RPC_H
