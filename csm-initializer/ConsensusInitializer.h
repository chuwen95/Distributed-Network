//
// Created by ChuWen on 2024/6/22.
//

#ifndef CONSENSUSINITIALIZER_H
#define CONSENSUSINITIALIZER_H

#include "csm-common/Common.h"
#include "csm-tool/NodeConfig.h"
#include "../csm-service/service/P2PService.h"
#include "csm-stmclog/StateMachineLog.h"
#include "csm-consensus/csm-raft/RaftFactory.h"

namespace csm
{

    namespace initializer
    {

        class ConsensusInitializer
        {
        public:
            using Ptr = std::shared_ptr<ConsensusInitializer>;

            explicit ConsensusInitializer(tool::NodeConfig::Ptr nodeConfig,
                service::P2PService* tcpService, storage::Storage::Ptr storage, stmclog::StateMachineLog::Ptr stateMachineLog);
            ~ConsensusInitializer() = default;

            int init();

            int start();

            int stop();

            consensus::Raft::Ptr raft();

        private:
            consensus::Raft::Ptr m_raft;
        };

    }

}

#endif //CONSENSUSINITIALIZER_H
