//
// Created by Chu on 2024/9/4.
//

#ifndef HTTPRPCSERVER_H
#define HTTPRPCSERVER_H

#include "csm-common/Common.h"

#include <httplib.h>

#include "csm-rpc/framework/RpcServer.h"
#include "csm-service/service/P2PService.h"

namespace csm
{

    namespace rpc
    {

        class HttpRpcServer : public RpcServer
        {
        public:
            explicit HttpRpcServer(tool::NodeConfig* nodeConfig, std::unique_ptr<httplib::Server> server,
                                   service::P2PService* service);
            ~HttpRpcServer() override = default;

        public:
            int init() override;

            int start() override;

            int stop() override;

        private:
            tool::NodeConfig* m_nodeConfig{nullptr};
            std::unique_ptr<httplib::Server> m_httpServer{nullptr};
            service::P2PService* m_p2pService{nullptr};
        };

    }

}

#endif //HTTPRPCSERVER_H