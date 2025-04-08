//
// Created by ChuWen on 2024/6/5.
//

#include "Raft.h"

#include "csm-stmclog/StateMachineLog.h"
#include "csm-tool/ClusterConfigurationSerializer.h"
#include "csm-utilities/RandomNumber.h"
#include "csm-utilities/TimeTools.h"

using namespace csm::consensus;

Raft::Raft(RaftConfig::Ptr raftConfig) : m_raftConfig(raftConfig)
{}

int Raft::init()
{
    if (-1 == initClusterConfiguration())
    {
        LOG->write(utilities::LogType::Log_Error, FILE_INFO, "init cluster configuration failed");
        return -1;
    }
    LOG->write(utilities::LogType::Log_Info, FILE_INFO, "init cluster configuration successfully");

    if (-1 == initState())
    {
        LOG->write(utilities::LogType::Log_Error, FILE_INFO, "init state failed");
        return -1;
    }
    LOG->write(utilities::LogType::Log_Info, FILE_INFO, "init state successfully");

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
    std::uint64_t endLogIndex = m_raftConfig->stateMachineLog()->endIndex();
    if(0 == endLogIndex)
    {
        // 日志不存在，创建
        if (-1 == m_raftConfig->stateMachineLog()->buildInitialConfigurationLog())
        {
            LOG->write(utilities::LogType::Log_Error, FILE_INFO, "build cluster initial configuration failed");
            return -1;
        }
    }
    else
    {
        for (int i = 1; i <= endLogIndex; ++i)
        {
            csm::stmclog::EntryType type = m_raftConfig->stateMachineLog()->getLogType(i);
            if (csm::stmclog::EntryType::Configuration == type)
            {
                csm::stmclog::Entry::Ptr entry = m_raftConfig->stateMachineLog()->getLogEntry(i);

                std::vector<std::string> clusterServerIds = csm::tool::ClusterConfigurationSerializer::deserialize(entry->data().data());
                ClusterConfiguration::Ptr clusterConfiguration = std::make_shared<ClusterConfiguration>(clusterServerIds);

                m_raftConfig->clusterConfigurationManager()->addClusterConfiguration(i, clusterConfiguration);
            }
        }
    }

    return 0;
}

int Raft::initState()
{
    if (false == m_raftConfig->persistentState()->init())
    {
        LOG->write(utilities::LogType::Log_Error, FILE_INFO, "init persistent state failed");
        return -1;
    }
    LOG->write(utilities::LogType::Log_Info, FILE_INFO, "init persistent state successfully");

    return 0;
}

int Raft::initElectionThread()
{
    // 计算下一个选举超时时间点
    m_electionTimeoutPoint = utilities::TimeTools::upTime() + generateRandomElectionTimeout();

    const auto electionThread = [this]() {
        switch (m_nodeIdentity)
        {
        case NodeIdentity::Leader:      // 如果本节点是领导者，则选举线程不用工作，等待节点身份变为非领导者
            {
                std::unique_lock<std::mutex> ulock(x_election);
                m_electionCv.wait(ulock, [this](){ return NodeIdentity::Leader != m_nodeIdentity; });
            }
            break;
        case NodeIdentity::Follower:
            {
                if (utilities::TimeTools::upTime() < m_electionTimeoutPoint)
                {
                    return;
                }

                // 开始一轮选举流程

            }
            break;
        case NodeIdentity::Candidate:
            {
                std::unique_lock<std::mutex> ulock(x_election);
                m_electionCv.wait(ulock, [this](){ return NodeIdentity::Leader != m_nodeIdentity; });
            }
            break;
        }
    };
    m_electionThread = std::make_shared<utilities::Thread>(electionThread, 10, "raft_vote");

    return 0;
}

int Raft::generateRandomElectionTimeout()
{
    return utilities::RandomNumber::lcrc(m_raftConfig->minElectionTimeout(), m_raftConfig->maxElectionTimeout());
}

int Raft::startElection()
{
    // 设置节点身份为候选人
    m_nodeIdentity = NodeIdentity::Candidate;
    // 任期+1
    m_raftConfig->persistentState()->increaseTerm();
    // 向其他节点发送RequestVote请求

    return 0;
}
