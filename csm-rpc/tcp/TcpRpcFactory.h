//
// Created by ChuWen on 10/19/23.
//

#ifndef TCPNETWORK_TCPRPCFACTORY_H
#define TCPNETWORK_TCPRPCFACTORY_H

#include "csm-rpc/framework/RpcFactory.h"
#include "csm-service/service/P2PService.h"
#include "csm-rpc/framework/RpcServer.h"

namespace csm
{

    namespace rpc
    {

        class TcpRpcFactory : public RpcFactory
        {
        public:
            explicit TcpRpcFactory(tool::NodeConfig* nodeConfig);
            ~TcpRpcFactory() override = default;

        public:
            std::unique_ptr<RpcServer> create() override;

        private:
            tool::NodeConfig* m_nodeConfig{nullptr};
        };

    } // rpc

}

#endif //TCPNETWORK_RPCFACTORY_H
