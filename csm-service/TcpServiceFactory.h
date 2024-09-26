//
// Created by ChuWen on 10/11/23.
//

#ifndef TCPNETWORK_TCPSERVICEFACTORY_H
#define TCPNETWORK_TCPSERVICEFACTORY_H

#include "config/ServiceConfig.h"
#include "TcpService.h"

namespace csm
{

    namespace service
    {

        class TcpServiceFactory
        {
        public:
            using Ptr = std::shared_ptr<TcpServiceFactory>;

            TcpServiceFactory(tool::NodeConfig::Ptr nodeConfig);
            ~TcpServiceFactory() = default;

        public:
            TcpService::Ptr createTcpService(const ServiceStartType serviceStartType);

        private:
            tool::NodeConfig::Ptr m_nodeConfig;
        };

    } // service

}

#endif //TCPNETWORK_TCPSERVICEFACTORY_H
