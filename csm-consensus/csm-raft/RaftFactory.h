//
// Created by ChuWen on 2024/6/4.
//

#ifndef COPYSTATEMACHINE_RAFTFACTORY_H
#define COPYSTATEMACHINE_RAFTFACTORY_H

#include "csm-consensus/csm-raft/Raft.h"
#include "csm-tool/NodeConfig.h"
#include "../../csm-service/service/P2PService.h"
#include "csm-storage/Storage.h"
#include "csm-stmclog/StateMachineLog.h"

namespace csm
{

    namespace consensus
    {

        class RaftFactory
        {
        public:
            using Ptr = std::shared_ptr<RaftFactory>;

            explicit RaftFactory(tool::NodeConfig::Ptr nodeConfig,
                service::P2PService* tcpService, storage::Storage::Ptr storage, stmclog::StateMachineLog::Ptr stateMachineLog);
            ~RaftFactory() = default;

        public:
            Raft::Ptr create();

        private:
            tool::NodeConfig::Ptr m_nodeConfig;
            service::P2PService* m_tcpService;
            storage::Storage::Ptr m_storage;
            stmclog::StateMachineLog::Ptr m_stateMachineLog;
        };

    };

}

#endif //COPYSTATEMACHINE_RAFTFACTORY_H
