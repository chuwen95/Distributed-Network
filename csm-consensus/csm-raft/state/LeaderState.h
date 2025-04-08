//
// Created by chu on 3/18/25.
//

#ifndef LEADERSTATE_H
#define LEADERSTATE_H

#include "csm-framework/consensus/raft/Common.h"

namespace csm
{

    namespace consensus
    {
        /**
         * Leader的不稳定状态，重新选举后
         */
        class LeaderState
        {
        public:
            using Ptr = std::shared_ptr<LeaderState>;

            LeaderState() = default;
            ~LeaderState() = default;

        public:
            std::vector<std::uint64_t> logIndexNextSend() const;
            std::vector<std::uint64_t> logIndexMatch() const;

        private:
            // 对于每个节点，下一个要发送给服务器的日志条目的索引，初始化后为领导者最后的日志索引+1
            std::vector<std::uint64_t> m_logIndexNextSend;
            // 对于每个服务器，已知在该服务器上被复制的最高日志条目的索引（初始化为0,单调递增）
            std::vector<std::uint64_t> m_logIndexMatch;
        };

    }

}



#endif //LEADERSTATE_H
