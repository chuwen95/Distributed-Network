//
// Created by ChuWen on 10/11/23.
//

#ifndef TCPNETWORK_P2PSERVICEINITIALIZER_H
#define TCPNETWORK_P2PSERVICEINITIALIZER_H

#include "csm-tool/NodeConfig.h"
#include "csm-service/service/P2PService.h"

namespace csm
{

    namespace initializer
    {

        class P2PServiceInitializer
        {
        public:
            using Ptr = std::shared_ptr<P2PServiceInitializer>;

            explicit P2PServiceInitializer(tool::NodeConfig* nodeConfig);
            ~P2PServiceInitializer() = default;

        public:
            int init();

            int start();

            int stop();

            service::P2PService* p2pService();

        private:
            std::unique_ptr<service::P2PService> m_p2pService;
        };

    } // initializer

}

#endif //TCPNETWORK_P2PSERVICEINITIALIZER_H
