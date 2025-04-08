//
// Created by ChuWen on 2024/6/4.
//

#include "RaftConfig.h"

using namespace csm::consensus;

RaftConfig::RaftConfig(const NodeId &id, std::int32_t minElectionTimeout, std::int32_t maxElectionTimeout, service::TcpService::Ptr service,
    PersistentState::Ptr persistentState, VolatileState::Ptr volatileState, LeaderState::Ptr leaderState,
    ClusterConfigurationManager::Ptr clusterConfigurationManager, stmclog::StateMachineLog::Ptr stateMachineLog) :
    m_id(id), m_minElectionTimeout(minElectionTimeout), m_maxElectionTimeout(maxElectionTimeout), m_tcpService(std::move(service)),
    m_persistentState(std::move(persistentState)), m_volatileState(std::move(volatileState)), m_leaderState(std::move(leaderState)),
    m_clusterConfigurationManager(std::move(clusterConfigurationManager)), m_stateMachineLog(std::move(stateMachineLog))
{ }

const NodeId& RaftConfig::nodeId()
{
    return m_id;
}

std::int32_t RaftConfig::minElectionTimeout()
{
    return m_minElectionTimeout;
}

std::int32_t RaftConfig::maxElectionTimeout()
{
    return m_maxElectionTimeout;
}

csm::service::TcpService::Ptr RaftConfig::tcpService()
{
    return m_tcpService;
}

PersistentState::Ptr RaftConfig::persistentState()
{
    return m_persistentState;
}

VolatileState::Ptr RaftConfig::volatileState()
{
    return m_volatileState;
}

LeaderState::Ptr RaftConfig::leaderState()
{
    return m_leaderState;
}

ClusterConfigurationManager::Ptr RaftConfig::clusterConfigurationManager()
{
    return m_clusterConfigurationManager;
}

csm::stmclog::StateMachineLog::Ptr RaftConfig::stateMachineLog()
{
    return m_stateMachineLog;
}
