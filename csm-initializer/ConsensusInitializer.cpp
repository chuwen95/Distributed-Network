//
// Created by ChuWen on 2024/6/22.
//

#include "ConsensusInitializer.h"

#include "csm-consensus/csm-raft/RaftFactory.h"

using namespace csm::initializer;

ConsensusInitializer::ConsensusInitializer(tool::NodeConfig::Ptr nodeConfig,
    service::TcpService::Ptr tcpService, storage::Storage::Ptr storage, stmclog::StateMachineLog::Ptr stateMachineLog)
{
    consensus::RaftFactory::Ptr raftFactory = std::make_shared<consensus::RaftFactory>(
            std::move(nodeConfig), tcpService, storage, stateMachineLog);
    m_raft = raftFactory->createRaft();
}

int ConsensusInitializer::init()
{
    return m_raft->init();
}

int ConsensusInitializer::start()
{
    return m_raft->start();
}

int ConsensusInitializer::stop()
{
    return m_raft->stop();
}
