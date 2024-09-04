//
// Created by ChuWen on 10/11/23.
//

#ifndef TCPNETWORK_TCPSERVICEINITIALIZER_H
#define TCPNETWORK_TCPSERVICEINITIALIZER_H

#include "csm-common/Common.h"

#include "csm-tool/NodeConfig.h"
#include "csm-service/TcpService.h"

namespace csm
{

    namespace initializer
    {

        class TcpServiceInitializer
        {
        public:
            using Ptr = std::shared_ptr<TcpServiceInitializer>;

            TcpServiceInitializer(tool::NodeConfig::Ptr nodeConfig);
            ~TcpServiceInitializer() = default;

        public:
            int init();

            int start();

            int stop();

            service::TcpService::Ptr tcpService();

        private:
            service::TcpService::Ptr m_tcpService;
        };

    } // initializer

}

#endif //TCPNETWORK_TCPSERVICEINITIALIZER_H
