//
// Created by root on 10/19/23.
//

#ifndef TCPNETWORK_RPCCONFIG_H
#define TCPNETWORK_RPCCONFIG_H

#include "libservice/TcpService.h"
#include "libtools/NodeConfig.h"
#include "httplib.h"

namespace rpc
{

    class RpcConfig
    {
    public:
        using Ptr = std::shared_ptr<RpcConfig>;

        RpcConfig(tool::NodeConfig::Ptr nodeConfig, service::TcpService::Ptr tcpService,
                  std::shared_ptr<httplib::Server> httpServer);
        ~RpcConfig() = default;

        tool::NodeConfig::Ptr nodeConfig();

        service::TcpService::Ptr tcpService();

        std::shared_ptr<httplib::Server> httpServer();

    private:
        service::TcpService::Ptr m_tcpService;
        tool::NodeConfig::Ptr m_nodeConfig;

        std::shared_ptr<httplib::Server> m_httpServer;
    };

} // rpc

#endif //TCPNETWORK_RPCCONFIG_H
