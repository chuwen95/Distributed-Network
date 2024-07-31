//
// Created by ChuWen on 2024/6/4.
//

#include "RaftFactory.h"
#include "csm-consensus/csm-raft/RaftConfig.h"

using namespace csm::consensus;

RaftFactory::RaftFactory(const std::string& id, const std::vector<std::string> &clusterServerIds, stmclog::StateMachineLog::Ptr stateMachineLog) :
    m_id(id),
    m_clusterServerIds(clusterServerIds),
    m_stateMachineLog(std::move(stateMachineLog))
{ }

csm::consensus::Raft::Ptr RaftFactory::createRaft()
{
    RaftConfig::Ptr raftConfig = std::make_shared<RaftConfig>(m_id, m_clusterServerIds, m_stateMachineLog);

    return std::make_shared<Raft>(raftConfig);
}
