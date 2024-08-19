//
// Created by ChuWen on 2024/6/4.
//

#ifndef COPYSTATEMACHINE_RAFTCONFIG_H
#define COPYSTATEMACHINE_RAFTCONFIG_H

#include "csm-common/Common.h"
#include "csm-stmclog/StateMachineLog.h"
#include "configuration/ClusterConfigurationManager.h"

namespace csm
{

    namespace consensus
    {

        class RaftConfig
        {
        public:
            using Ptr = std::shared_ptr<RaftConfig>;

            RaftConfig(const std::string &id, stmclog::StateMachineLog::Ptr stateMachineLog, ClusterConfigurationManager::Ptr clusterConfigurationManager);
            ~RaftConfig() = default;

            /**
             * 获取本服务器ID
             * @return
             */
            const std::string& id();

            /**
             *  获取StateMachineLog对象
             * @return
             */
            stmclog::StateMachineLog::Ptr stateMachineLog();

            /**
             * 获取集群配置管理器
             * @return
             */
            ClusterConfigurationManager::Ptr clusterConfigurationManager();

        private:
            std::string m_id;
            stmclog::StateMachineLog::Ptr m_stateMachineLog;
            ClusterConfigurationManager::Ptr m_clusterConfigurationManager;
        };

    }

}

#endif //COPYSTATEMACHINE_RAFTCONFIG_H
