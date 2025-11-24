//
// Created by ChuWen on 2024/2/25.
//

#ifndef TCPNETWORK_APPLOGINITIALIZER_H
#define TCPNETWORK_APPLOGINITIALIZER_H

#include "csm-utilities/Logger.h"
#include "csm-tool/NodeConfig.h"

namespace csm
{

    namespace initializer
    {

        class AppLogInitializer
        {
        public:
            using Ptr = std::shared_ptr<AppLogInitializer>;

            AppLogInitializer(tool::NodeConfig* nodeConfig);
            ~AppLogInitializer() = default;

        public:
            int init();

            int start();

            int stop();

        private:
            tool::NodeConfig* m_nodeConfig;
        };

    } // initializer

}

#endif //TCPNETWORK_APPLOGINITIALIZER_H
