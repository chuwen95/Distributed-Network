//
// Created by chu on 3/18/25.
//

#ifndef PERSISTENTSTATE_H
#define PERSISTENTSTATE_H

#include "csm-framework/cluster/Common.h"
#include "csm-consensus/csm-raft/storage/RaftMetadataStorage.h"
#include "csm-stmclog/protocol/packet/Entry.h"

namespace csm
{

    namespace consensus
    {
        /**
         * 所有服务器的持久状态（持久状态即存储在磁盘上的状态，RPC回应应在更新持久状态后发生）
         */
        class PersistentState
        {
        public:
            using Ptr = std::shared_ptr<PersistentState>;

            PersistentState(RaftMetadataStorage::Ptr storage);
            ~PersistentState() = default;

        public:
            int init();

        public:
            [[nodiscard]] int currentTerm(std::uint64_t& term) const;
            int setCurrentTerm(std::uint64_t term);
            int increaseTerm();

            [[nodiscard]] NodeId voteFor() const;
            void setVoteFor(const NodeId& voteFor);

            void addEntry(stmclog::Entry::Ptr entry);

        private:
            // 当前任期
            std::atomic_uint64_t m_currentTerm{ 0 };
            // 当前任期内本节点将票投给了谁
            NodeId m_voteFor;
            // 本节点的日志条目集合
            std::queue<stmclog::Entry::Ptr> m_logs;

            RaftMetadataStorage::Ptr m_raftMetadataStorage;
        };

    }

}

#endif //PERSISTENTSTATE_H
