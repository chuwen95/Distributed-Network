//
// Created by ChuWen on 2024/6/22.
//

#include "ConsensusInitializer.h"

#include "csm-consensus/csm-raft/RaftFactory.h"

using namespace csm::initializer;

ConsensusInitializer::ConsensusInitializer(csm::tool::NodeConfig::Ptr nodeConfig, stmclog::StateMachineLog::Ptr stateMachineLog) :
    m_nodeConfig(std::move(nodeConfig))
{
    consensus::RaftFactory::Ptr raftFactory = std::make_shared<consensus::RaftFactory>(
            m_nodeConfig->id(), m_nodeConfig->clusterServerIds(), stateMachineLog);
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
