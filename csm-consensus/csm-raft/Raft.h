//
// Created by ChuWen on 2024/6/4.
//

#ifndef RAFT_H
#define RAFT_H

#include "csm-common/Common.h"
#include "csm-utilities/Thread.h"

#include "RaftConfig.h"

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
            RaftConfig::Ptr m_raftConfig;

            csm::utilities::Thread::Ptr m_voteThread;
        };

    }

}

#endif //RAFT_H
