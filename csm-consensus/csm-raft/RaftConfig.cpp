//
// Created by ChuWen on 2024/6/4.
//

#include "RaftConfig.h"

using namespace csm::consensus;

RaftConfig::RaftConfig(const std::string &id, const std::vector<std::string> &clusterServerIds, stmclog::StateMachineLog::Ptr stateMachineLog) :
    m_id(id),
    m_clusterServerIds(clusterServerIds),
    m_stateMachineLog(std::move(stateMachineLog))
{ }

const std::string& RaftConfig::id()
{
    return m_id;
}

const std::vector<std::string>& RaftConfig::clusterServerIds() const
{
    return m_clusterServerIds;
}

csm::stmclog::StateMachineLog::Ptr RaftConfig::stateMachineLog()
{
    return m_stateMachineLog;
}
