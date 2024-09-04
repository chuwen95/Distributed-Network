//
// Created by ChuWen on 10/11/23.
//

#ifndef TCPNETWORK_INITIALIZER_H
#define TCPNETWORK_INITIALIZER_H

#include "csm-common/Common.h"

#include "StorageInitializer.h"
#include "AppLogInitializer.h"
#include "TcpServiceInitializer.h"
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

            Initializer() = default;
            ~Initializer() = default;

        public:
            int initConfig(const std::string &configPath);

            int init();

            int start();

            int stop();

        public:
            tool::NodeConfig::Ptr m_nodeConfig;

            StorageInitializer::Ptr m_storageInitializer;
            AppLogInitializer::Ptr m_logInitializer;
            TcpServiceInitializer::Ptr m_tcpServiceInitializer;
            RpcInitializer::Ptr m_rpcInitializer;
            ConsensusInitializer::Ptr m_consensusInitializer;
        };

    } // initializer

}

#endif //TCPNETWORK_INITIALIZER_H
