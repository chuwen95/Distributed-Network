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

        private:
            RaftConfig::Ptr m_raftConfig;

            enum class NodeIdentity
            {
                Follower,
                Candidate,
                Leader,
            };
            std::atomic<NodeIdentity> m_nodeIdentity{ NodeIdentity::Follower };

            std::mutex x_election;
            std::condition_variable m_electionCv;
            // 选举超时时间点
            double m_electionTimeoutPoint;
            // 选举线程
            utilities::Thread::Ptr m_electionThread;
        };

    }

}

#endif //RAFT_H
