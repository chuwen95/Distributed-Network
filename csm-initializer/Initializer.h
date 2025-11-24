//
// Created by ChuWen on 10/11/23.
//

#ifndef TCPNETWORK_INITIALIZER_H
#define TCPNETWORK_INITIALIZER_H

#include "csm-common/Common.h"

#include "StorageInitializer.h"
#include "AppLogInitializer.h"
#include "P2PServiceInitializer.h"
#include "RpcInitializer.h"
#include "ConsensusInitializer.h"

namespace csm
{

    namespace initializer
    {

        class Initializer
        {
        public:
            using Ptr = std::shared_ptr<Initializer>;

            explicit Initializer(std::string configPath);

        public:
            int init();

            int start();

            int stop();

        private:
            int initP2PServiceModuleMessageHandler();

        private:
            std::string m_configPath;
            std::unique_ptr<tool::NodeConfig> m_nodeConfig;

            std::unique_ptr<StorageInitializer> m_storageInitializer;
            std::unique_ptr<AppLogInitializer> m_logInitializer;
            std::unique_ptr<P2PServiceInitializer> m_p2pServiceInitializer;
            std::unique_ptr<RpcInitializer> m_rpcInitializer;
            std::unique_ptr<ConsensusInitializer> m_consensusInitializer;
        };

    } // initializer

}

#endif //TCPNETWORK_INITIALIZER_H
