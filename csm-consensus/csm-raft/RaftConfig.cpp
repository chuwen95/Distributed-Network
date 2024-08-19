//
// Created by ChuWen on 2024/6/4.
//

#include "RaftConfig.h"

using namespace csm::consensus;

RaftConfig::RaftConfig(const std::string &id, stmclog::StateMachineLog::Ptr stateMachineLog, ClusterConfigurationManager::Ptr clusterConfigurationManager) :
    m_id(id),
    m_stateMachineLog(std::move(stateMachineLog)),
    m_clusterConfigurationManager(std::move(clusterConfigurationManager))
{ }

const std::string& RaftConfig::id()
{
    return m_id;
}

csm::stmclog::StateMachineLog::Ptr RaftConfig::stateMachineLog()
{
    return m_stateMachineLog;
}

ClusterConfigurationManager::Ptr RaftConfig::clusterConfigurationManager()
{
    return m_clusterConfigurationManager;
}
