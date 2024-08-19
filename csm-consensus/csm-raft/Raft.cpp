//
// Created by ChuWen on 2024/6/5.
//

#include "Raft.h"

using namespace csm::consensus;

Raft::Raft(RaftConfig::Ptr raftConfig) : m_raftConfig(raftConfig)
{}

int Raft::init()
{
    return 0;
}

int Raft::start()
{
    return 0;
}

int Raft::stop()
{
    return 0;
}

int Raft::initClusterConfiguration()
{
    std::uint64_t beginLogIndex = m_raftConfig->stateMachineLog()->beginIndex();
    std::uint64_t endLogIndex = m_raftConfig->stateMachineLog()->endIndex();
    if(-1 == beginLogIndex || -1 == endLogIndex)
    {
        // 日志不存在，创建
        m_raftConfig->stateMachineLog()->buildInitialConfigurationLog();
    }




    return 0;
}

