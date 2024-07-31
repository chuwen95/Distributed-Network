//
// Created by ChuWen on 2024/6/4.
//

#ifndef COPYSTATEMACHINE_RAFTFACTORY_H
#define COPYSTATEMACHINE_RAFTFACTORY_H

#include "csm-common/Common.h"
#include "csm-consensus/csm-raft/Raft.h"
#include "csm-stmclog/StateMachineLog.h"

namespace csm
{

    namespace consensus
    {

        class RaftFactory
        {
        public:
            using Ptr = std::shared_ptr<RaftFactory>;

            explicit RaftFactory(const std::string& id, const std::vector<std::string>& clusterServerIds, stmclog::StateMachineLog::Ptr stateMachineLog);
            ~RaftFactory() = default;

        public:
            Raft::Ptr createRaft();

        private:
            std::string m_id;
            std::vector<std::string> m_clusterServerIds;
            stmclog::StateMachineLog::Ptr m_stateMachineLog;
        };

    };

}

#endif //COPYSTATEMACHINE_RAFTFACTORY_H
