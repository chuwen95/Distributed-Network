//
// Created by ChuWen on 10/11/23.
//

#ifndef TCPNETWORK_P2PSERVICEINITIALIZER_H
#define TCPNETWORK_P2PSERVICEINITIALIZER_H

#include "csm-common/Common.h"

#include "csm-tool/NodeConfig.h"
#include "csm-service/P2PService.h"

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

            service::P2PService::Ptr p2pService();

        private:
            service::P2PService::Ptr m_p2pService;
        };

    } // initializer

}

#endif //TCPNETWORK_P2PSERVICEINITIALIZER_H
