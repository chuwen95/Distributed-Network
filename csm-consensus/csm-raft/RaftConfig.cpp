//
// Created by ChuWen on 2024/6/4.
//

#include "RaftConfig.h"

using namespace csm::consensus;

RaftConfig::RaftConfig(const NodeId &nodeId, std::int32_t minElectionTimeout, std::int32_t maxElectionTimeout, service::P2PService::Ptr p2pService,
    PersistentState::Ptr persistentState, VolatileState::Ptr volatileState, LeaderState::Ptr leaderState,
    ClusterConfigurationManager::Ptr clusterConfigurationManager, stmclog::StateMachineLog::Ptr stateMachineLog) :
    m_nodeId(nodeId), m_minElectionTimeout(minElectionTimeout), m_maxElectionTimeout(maxElectionTimeout), m_p2pService(std::move(p2pService)),
    m_persistentState(std::move(persistentState)), m_volatileState(std::move(volatileState)), m_leaderState(std::move(leaderState)),
    m_clusterConfigurationManager(std::move(clusterConfigurationManager)), m_stateMachineLog(std::move(stateMachineLog))
{ }

const NodeId& RaftConfig::nodeId()
{
    return m_nodeId;
}

void RaftConfig::setNodeIndex(std::uint32_t nodeIndex)
{
    m_nodeIndex = nodeIndex;
}

std::uint32_t RaftConfig::nodeIndex() const
{
    return m_nodeIndex;
}

std::int32_t RaftConfig::minElectionTimeout()
{
    return m_minElectionTimeout;
}

std::int32_t RaftConfig::maxElectionTimeout()
{
    return m_maxElectionTimeout;
}

csm::service::P2PService::Ptr RaftConfig::p2pService()
{
    return m_p2pService;
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
