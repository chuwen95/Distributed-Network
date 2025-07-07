//
// Created by ChuWen on 2024/6/4.
//

#ifndef RAFT_H
#define RAFT_H

#include "csm-common/Common.h"
#include "csm-utilities/Thread.h"

#include "RaftConfig.h"
#include "state/PersistentState.h"
#include "state/VolatileState.h"
#include "state/LeaderState.h"

#include "protocol/packet/RequestVoteMessage.h"
#include "protocol/packet/RequestVoteReplyMessage.h"

namespace csm
{

    namespace consensus
    {

        class Raft
        {
        public:
            using Ptr = std::shared_ptr<Raft>;

            Raft(RaftConfig::Ptr raftConfig);
            ~Raft() = default;

        public:
            int init();

            int start();

            int stop();

            int handleMessage(const NodeId& fromNodeId, const std::vector<char>& messageData);

        private:
            // 初始化集群配置
            int initClusterConfiguration();
            // 初始化状态
            int initState();
            // 初始化投票线程
            int initElectionThread();

            // 生成随机的选举超时时间
            int generateRandomElectionTimeout();
            // 开始选举流程
            int startElection();

            // 构建RequestVote消息
            RequestVoteMessage::Ptr generateRequestVoteMessage();

            // 广播消息给所有其他节点
            int boardcastMessage(MessageBase::Ptr message);
            // 发送消息给某个节点
            int sendMessageToNode(const NodeId& targetNodeId, MessageBase::Ptr message);

            // 处理RequestVote消息
            int handleRequestVoteMessage(const NodeId& fromNode, RequestVoteMessage::Ptr msg);

        private:
            RaftConfig::Ptr m_raftConfig;

            enum class NodeIdentity
            {
                Follower,
                Candidate,
                Leader,
            };
            std::atomic<NodeIdentity> m_nodeIdentity{ NodeIdentity::Follower };

            // 选举超时时间点
            std::atomic<double> m_electionTimeoutPoint;
            // 选举线程
            utilities::Thread::Ptr m_electionThread;
        };

    }

}

#endif //RAFT_H