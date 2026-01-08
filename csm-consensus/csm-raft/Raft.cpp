//
// Created by ChuWen on 2024/6/5.
//

#include "Raft.h"

#include "csm-stmclog/StateMachineLog.h"
#include "csm-tool/ClusterConfigurationSerializer.h"
#include "csm-utilities/RandomNumber.h"
#include "csm-utilities/TimeTools.h"
#include "csm-consensus/csm-raft/protocol/utilities/MessageEncodeHelper.h"
#include "csm-framework/protocol/Protocol.h"
#include "csm-consensus/csm-raft/protocol/packet/RaftMessagePack.h"
#include "csm-consensus/csm-raft/protocol/packet/RequestVoteMessage.h"

using namespace csm::consensus;

Raft::Raft(RaftConfig::Ptr raftConfig) : m_raftConfig(std::move(raftConfig))
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

int Raft::handleMessage(const NodeId& fromNodeId, const std::vector<char>& messageData)
{
    RaftMessagePack raftMessage;
    raftMessage.decode(const_cast<char*>(messageData.data()), messageData.size());

    switch (raftMessage.messageType())
    {
    case RaftMessageType::RequestVote:
        {
            RequestVoteMessage::Ptr requestVoteMsg = std::make_shared<RequestVoteMessage>();
            requestVoteMsg->decode(raftMessage.payloadRawPointer(), raftMessage.payloadSize());

            if (0 != handleRequestVoteMessage(fromNodeId, requestVoteMsg))
            {
                LOG->write(utilities::LogType::Log_Error, FILE_INFO, "handle request vote message failed");
                return -1;
            }
        }
        break;
    case RaftMessageType::RequestVoteReply:
        break;
    default:
        LOG->write(utilities::LogType::Log_Error, FILE_INFO, "Raft message type not supported");
        break;
    }

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
        LOG->write(utilities::LogType::Log_Info, FILE_INFO, "build cluster initial configuration successfully");
    }

    endLogIndex = m_raftConfig->stateMachineLog()->endIndex();
    for (int i = 1; i <= endLogIndex; ++i)
    {
        std::shared_ptr<std::vector<std::string>> clusterServerIds;

        stmclog::EntryType type = m_raftConfig->stateMachineLog()->getLogType(i);
        if (stmclog::EntryType::Configuration == type)
        {
            stmclog::Entry::Ptr entry = m_raftConfig->stateMachineLog()->getLogEntry(i);

            clusterServerIds = tool::ClusterConfigurationSerializer::deserialize(entry->data().data());
            ClusterConfiguration::Ptr clusterConfiguration = std::make_shared<ClusterConfiguration>(*clusterServerIds);

            m_raftConfig->clusterConfigurationManager()->addClusterConfiguration(i, clusterConfiguration);
        }
    }

    //


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

    const auto electionThread = [this](const std::stop_token& st) {
        switch (m_nodeIdentity)
        {
        case NodeIdentity::Leader:      // 如果本节点是领导者，则选举线程不用工作，等待节点身份变为非领导者
            break;
        case NodeIdentity::Follower:
            {
                if (utilities::TimeTools::upTime() < m_electionTimeoutPoint)
                {
                    return;
                }

                // 开始选举
                startElection();
            }
            break;
        case NodeIdentity::Candidate:
            {
                if (utilities::TimeTools::upTime() >= m_electionTimeoutPoint)
                {
                    // 如果超过了选举超时时间本节点还是候选人，即没有收到领导者心跳，自己也没有获得足够的投票成为领导者，那么更新选举超时时间并开启新一轮选举
                    m_electionTimeoutPoint += generateRandomElectionTimeout();
                    startElection();
                }
            }
            break;
        }
    };
    m_electionThread = std::make_unique<utilities::Thread>(electionThread, 10, "raft_vote");

    return 0;
}

int Raft::generateRandomElectionTimeout()
{
    return utilities::RandomNumber::lcrc(m_raftConfig->minElectionTimeout(), m_raftConfig->maxElectionTimeout());
}

int Raft::startElection()
{
    // 任期+1
    if (0 != m_raftConfig->persistentState()->increaseTerm())
    {
        LOG->write(utilities::LogType::Log_Error, FILE_INFO, "persistent state increase term failed");
        return -1;
    }
    // 转变自身身份为候选人
    m_nodeIdentity = NodeIdentity::Candidate;
    // 为自己投票
    ClusterNode::Ptr clusterNode = m_raftConfig->clusterConfigurationManager()->lastConfiguration()->clusterNode(m_raftConfig->nodeId());
    clusterNode->setIsVote(true);
    // 重置计时器
    m_electionTimeoutPoint += generateRandomElectionTimeout();
    // 生成RequestVote消息
    RequestVoteMessage::Ptr requestVoteMsg = generateRequestVoteMessage();
    // 向其他节点发送RequestVote请求
    if (0 != boardcastMessage(requestVoteMsg))
    {
        LOG->write(utilities::LogType::Log_Error, FILE_INFO, "boardcast request vote message failed");
        return -1;
    }

    return 0;
}

RequestVoteMessage::Ptr Raft::generateRequestVoteMessage()
{
    // 生成RequestVoteMsg包
    RequestVoteMessage::Ptr requestVoteMsg = std::make_shared<RequestVoteMessage>();

    std::uint64_t currentTerm{ 0 };
    int ret = m_raftConfig->persistentState()->currentTerm(currentTerm);
    if (0 != ret)
    {
        LOG->write(utilities::LogType::Log_Error, FILE_INFO, "persistent state get current term failed");
        return nullptr;
    }

    requestVoteMsg->setTerm(currentTerm);
    requestVoteMsg->setCandidateId(m_raftConfig->nodeId());

    std::uint64_t endIndex = m_raftConfig->stateMachineLog()->endIndex();
    stmclog::Entry::Ptr entry = m_raftConfig->stateMachineLog()->getLogEntry(endIndex);

    requestVoteMsg->setLastLogIndex(endIndex);
    requestVoteMsg->setLastLogTerm(entry->term());

    return requestVoteMsg;
}

int Raft::boardcastMessage(MessageBase::Ptr message)
{
    std::vector<char> buffer;
    if (0 != MessageEncodeHelper::encode(message, buffer))
    {
        LOG->write(utilities::LogType::Log_Error, FILE_INFO, "encode message failed");
        return -1;
    }

    return m_raftConfig->p2pService()->boardcastModuleMessage(protocol::ModuleID::raft, buffer);
}

int Raft::sendMessageToNode(const NodeId& targetNodeId, MessageBase::Ptr message)
{
    std::vector<char> buffer;
    if (0 != MessageEncodeHelper::encode(message, buffer))
    {
        LOG->write(utilities::LogType::Log_Error, FILE_INFO, "encode message failed");
        return -1;
    }

    return m_raftConfig->p2pService()->sendModuleMessageByNodeId(targetNodeId, protocol::ModuleID::raft, buffer);
}

int Raft::handleRequestVoteMessage(const NodeId& fromNodeId, RequestVoteMessage::Ptr msg)
{
    /*
     * 1. 如果投票发起者的任期 < 本节点任期，则返回false
     * 2. 如果votedFor的值是null或者是CandidateId，表示该接收者从来没有投过票或者已经将票投给过请求发起者，
     *    并且候选人的日志至少与接收者的日志一样或更新，则同意投票
     */
    std::uint64_t currentTerm{ 0 };
    if (0 != m_raftConfig->persistentState()->currentTerm(currentTerm))
    {
        LOG->write(utilities::LogType::Log_Error, FILE_INFO, "get current term failed");
        return -1;
    }

    RequestVoteReplyMessage::Ptr requestVoteReplyMsg = std::make_shared<RequestVoteReplyMessage>();
    requestVoteReplyMsg->setTerm(currentTerm);
    requestVoteReplyMsg->setVoteGranted(false);

    NodeId voteFor = m_raftConfig->persistentState()->voteFor();
    if (msg->term() < currentTerm ||
        (false == voteFor.empty() && voteFor != m_raftConfig->nodeId()) || msg->lastLogIndex() < m_raftConfig->stateMachineLog()->endIndex())
    {
        // 如果投票发起者的任期 < 本节点任期，拒绝投票
        if (0 != sendMessageToNode(fromNodeId, requestVoteReplyMsg))
        {
            LOG->write(utilities::LogType::Log_Error, FILE_INFO, "send message failed");
            return -1;
        }

        LOG->write(utilities::LogType::Log_Info, FILE_INFO, "the term of node which send request vote is less than this node, reject vote");
        return 0;
    }

    return 0;
}
