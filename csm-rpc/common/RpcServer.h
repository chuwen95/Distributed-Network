//
// Created by Chu on 2024/9/5.
//

#ifndef RPCSERVER_H
#define RPCSERVER_H

#include "csm-common/Common.h"
#include "csm-rpc/config/RpcConfig.h"

namespace csm
{

    namespace rpc
    {

        class RpcServer
        {
        public:
            using Ptr = std::shared_ptr<RpcServer>;

            RpcServer(RpcConfig::Ptr rpcConfig) : m_rpcConfig(std::move(rpcConfig)) { }
            virtual ~RpcServer() = default;

        public:
            virtual int init() = 0;

            virtual int start() = 0;

            virtual int stop() = 0;

        protected:
            RpcConfig::Ptr m_rpcConfig;
        };

    }

}

#endif //RPCSERVER_H
