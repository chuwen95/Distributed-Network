//
// Created by ChuWen on 2024/6/4.
//

#include "RaftFactory.h"
#include "csm-consensus/csm-raft/RaftConfig.h"

using namespace csm::consensus;

RaftFactory::RaftFactory(const std::vector<std::string> &clusterServerIds) :
    m_clusterServerIds(clusterServerIds)
{ }

csm::consensus::Raft::Ptr RaftFactory::createRaft()
{
    RaftConfig::Ptr raftConfig = std::make_shared<RaftConfig>(m_clusterServerIds);

    return std::make_shared<Raft>(raftConfig);
}
