//
// Created by Chu on 2024/9/5.
//

#ifndef TCPRPCSERVER_H
#define TCPRPCSERVER_H

#include "csm-rpc/common/RpcServer.h"
#include "csm-service/TcpServiceFactory.h"

namespace csm
{

    namespace rpc
    {

        class TcpRpcServer : public RpcServer
        {
        public:
            using Ptr = std::shared_ptr<TcpRpcServer>;

            TcpRpcServer(RpcConfig::Ptr rpcConfig);
            ~TcpRpcServer() = default;

        public:
            int init() override;

            int start() override;

            int stop() override;

        private:
            service::TcpService::Ptr m_tcpService;
        };

    }

}

#endif //TCPRPCSERVER_H
