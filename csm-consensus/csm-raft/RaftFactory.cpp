//
// Created by ChuWen on 2024/6/4.
//

#include "RaftFactory.h"

#include "state/PersistentState.h"
#include "state/VolatileState.h"
#include "state/LeaderState.h"
#include "RaftConfig.h"

using namespace csm::consensus;

RaftFactory::RaftFactory(tool::NodeConfig::Ptr nodeConfig,
    service::TcpService::Ptr tcpService, storage::Storage::Ptr storage, stmclog::StateMachineLog::Ptr stateMachineLog) :
    m_nodeConfig(std::move(nodeConfig)),
    m_tcpService(std::move(tcpService)), m_storage(std::move(storage)), m_stateMachineLog(std::move(stateMachineLog))
{ }

Raft::Ptr RaftFactory::createRaft()
{
    ClusterConfigurationManager::Ptr clusterConfigurationManager = std::make_shared<ClusterConfigurationManager>();
    RaftMetadataStorage::Ptr raftMetadataStorage = std::make_shared<RaftMetadataStorage>(m_storage);

    PersistentState::Ptr persistentState = std::make_shared<PersistentState>(raftMetadataStorage);
    VolatileState::Ptr volatileState = std::make_shared<VolatileState>();
    LeaderState::Ptr leaderState = std::make_shared<LeaderState>();

    RaftConfig::Ptr raftConfig = std::make_shared<RaftConfig>(m_nodeConfig->id(),
        m_nodeConfig->minRandomVoteTimeout(), m_nodeConfig->maxRandomVoteTimeout(),
        m_tcpService, persistentState, volatileState, leaderState, clusterConfigurationManager, m_stateMachineLog);

    return std::make_shared<Raft>(raftConfig);
}
