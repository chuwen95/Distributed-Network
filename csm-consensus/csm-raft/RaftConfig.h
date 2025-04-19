//
// Created by ChuWen on 2024/6/4.
//

#ifndef COPYSTATEMACHINE_RAFTCONFIG_H
#define COPYSTATEMACHINE_RAFTCONFIG_H

#include "csm-framework/consensus/raft/Common.h"
#include "csm-service/P2PService.h"
#include "state/PersistentState.h"
#include "state/VolatileState.h"
#include "state/LeaderState.h"
#include "configuration/ClusterConfigurationManager.h"
#include "csm-stmclog/StateMachineLog.h"

namespace csm
{

    namespace consensus
    {

        class RaftConfig
        {
        public:
            using Ptr = std::shared_ptr<RaftConfig>;

            RaftConfig(const NodeId &nodeId, std::int32_t minElectionTimeout, std::int32_t maxElectionTimeout, service::P2PService::Ptr p2pService,
                PersistentState::Ptr persistentState, VolatileState::Ptr volatileState, LeaderState::Ptr leaderState,
                ClusterConfigurationManager::Ptr clusterConfigurationManager, stmclog::StateMachineLog::Ptr stateMachineLog);
            ~RaftConfig() = default;

            // 获取本节点ID
            const NodeId& nodeId();
            // 设置节点在节点列表中的index
            void setNodeIndex(std::uint32_t nodeIndex);
            // 获取节点在节点列表中的index
            std::uint32_t nodeIndex() const;

            // 最小选举超时
            std::int32_t minElectionTimeout();
            // 最大选举超时
            std::int32_t maxElectionTimeout();

            // P2P模块
            service::P2PService::Ptr p2pService();

            // 持久化数据
            PersistentState::Ptr persistentState();

            // 易失性数据
            VolatileState::Ptr volatileState();

            // 仅为状态才会用到的数据
            LeaderState::Ptr leaderState();

            // 获取集群配置管理器
            ClusterConfigurationManager::Ptr clusterConfigurationManager();

            // 获取StateMachineLog对象
            stmclog::StateMachineLog::Ptr stateMachineLog();

        private:
            // 本节点ID
            NodeId m_nodeId;
            // 本节点在节点列表中的index
            std::uint32_t m_nodeIndex;
            // 最小选举超时时间
            std::int32_t m_minElectionTimeout;
            // 最大选举超时时间
            std::int32_t m_maxElectionTimeout;

            // P2P模块
            service::P2PService::Ptr m_p2pService;

            /*
             * Raft元数据
             */
            // 持久化数据
            PersistentState::Ptr m_persistentState;
            // 易失性数据
            VolatileState::Ptr m_volatileState;
            // 仅为状态才会用到的数据
            LeaderState::Ptr m_leaderState;

            // 集群配置
            ClusterConfigurationManager::Ptr m_clusterConfigurationManager;
            // 复制状态机日志
            stmclog::StateMachineLog::Ptr m_stateMachineLog;
        };
    }

}

#endif //COPYSTATEMACHINE_RAFTCONFIG_H
