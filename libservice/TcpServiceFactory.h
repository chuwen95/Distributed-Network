//
// Created by root on 10/11/23.
//

#ifndef TCPNETWORK_TCPSERVICEFACTORY_H
#define TCPNETWORK_TCPSERVICEFACTORY_H

#include "config/ServiceConfig.h"
#include "TcpService.h"

namespace service
{

    class TcpServiceFactory
    {
    public:
        TcpServiceFactory(tool::NodeConfig::Ptr nodeConfig);
        ~TcpServiceFactory() = default;

    public:
        TcpService::Ptr createTcpService();

    private:
        tool::NodeConfig::Ptr m_nodeConfig;
    };

} // service

#endif //TCPNETWORK_TCPSERVICEFACTORY_H
