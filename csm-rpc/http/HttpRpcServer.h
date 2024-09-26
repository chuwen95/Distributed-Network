//
// Created by Chu on 2024/9/4.
//

#ifndef HTTPRPCSERVER_H
#define HTTPRPCSERVER_H

#include "csm-common/Common.h"
#include "csm-rpc/config/RpcConfig.h"
#include "csm-rpc/common/RpcServer.h"
#include <httplib.h>

namespace csm
{

    namespace rpc
    {

        class HttpRpcServer : public RpcServer
        {
        public:
            using Ptr = std::shared_ptr<HttpRpcServer>;

            HttpRpcServer(RpcConfig::Ptr rpcConfig);
            ~HttpRpcServer() = default;

        public:
            int init() override;

            int start() override;

            int stop() override;

        private:
            std::shared_ptr<httplib::Server> m_httpServer;
        };

    }

}

#endif //HTTPRPCSERVER_H