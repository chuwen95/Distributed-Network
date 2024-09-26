//
// Created by ChuWen on 10/11/23.
//

#ifndef TCPNETWORK_P2PSERVICEINITIALIZER_H
#define TCPNETWORK_P2PSERVICEINITIALIZER_H

#include "csm-common/Common.h"

#include "csm-tool/NodeConfig.h"
#include "csm-service/TcpService.h"

namespace csm
{

    namespace initializer
    {

        class P2PServiceInitializer
        {
        public:
            using Ptr = std::shared_ptr<P2PServiceInitializer>;

            P2PServiceInitializer(tool::NodeConfig::Ptr nodeConfig);
            ~P2PServiceInitializer() = default;

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

#endif //TCPNETWORK_P2PSERVICEINITIALIZER_H
