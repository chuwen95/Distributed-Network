//
// Created by Chu on 2024/9/5.
//

#ifndef TCPRPCSERVER_H
#define TCPRPCSERVER_H

#include "csm-rpc/framework/RpcServer.h"
#include "csm-service/factory/ServerServiceFactory.h"

namespace csm
{

    namespace rpc
    {

        class TcpRpcServer : public RpcServer
        {
        public:
            using Ptr = std::shared_ptr<TcpRpcServer>;

            explicit TcpRpcServer(std::unique_ptr<service::ServerService> service);
            ~TcpRpcServer() override = default;

        public:
            int init() override;

            int start() override;

            int stop() override;

        private:
            std::unique_ptr<service::ServerService> m_tcpService{nullptr};
        };

    }

}

#endif //TCPRPCSERVER_H
