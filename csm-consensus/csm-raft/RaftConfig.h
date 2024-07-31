//
// Created by ChuWen on 2024/6/4.
//

#ifndef COPYSTATEMACHINE_RAFTCONFIG_H
#define COPYSTATEMACHINE_RAFTCONFIG_H

#include "csm-common/Common.h"
#include "csm-stmclog/StateMachineLog.h"

namespace csm
{

    namespace consensus
    {

        class RaftConfig
        {
        public:
            using Ptr = std::shared_ptr<RaftConfig>;

            RaftConfig(const std::string &id, const std::vector<std::string>& clusterServerIds, stmclog::StateMachineLog::Ptr stateMachineLog);
            ~RaftConfig() = default;

            /**
             * 获取本服务器ID
             * @return
             */
            const std::string& id();

            /**
             * 获取集群中所有服务器ID（包含本服务器ID）
             * @return
             */
            const std::vector<std::string>& clusterServerIds() const;

            /**
             *  获取StateMachineLog对象
             * @return
             */
            stmclog::StateMachineLog::Ptr stateMachineLog();

        private:
            std::string m_id;
            std::vector<std::string> m_clusterServerIds;
            stmclog::StateMachineLog::Ptr m_stateMachineLog;
        };

    }

}

#endif //COPYSTATEMACHINE_RAFTCONFIG_H
