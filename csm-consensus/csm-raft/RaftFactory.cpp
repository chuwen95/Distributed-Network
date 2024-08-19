//
// Created by ChuWen on 2024/6/4.
//

#include "RaftFactory.h"
#include "RaftConfig.h"
#include "configuration/ClusterConfigurationManager.h"

using namespace csm::consensus;

RaftFactory::RaftFactory(const std::string& id, stmclog::StateMachineLog::Ptr stateMachineLog) :
    m_id(id),
    m_stateMachineLog(std::move(stateMachineLog))
{ }

csm::consensus::Raft::Ptr RaftFactory::createRaft()
{
    ClusterConfigurationManager::Ptr clusterConfigurationManager = std::make_shared<ClusterConfigurationManager>();
    RaftConfig::Ptr raftConfig = std::make_shared<RaftConfig>(m_id, m_stateMachineLog, clusterConfigurationManager);

    return std::make_shared<Raft>(raftConfig);
}
