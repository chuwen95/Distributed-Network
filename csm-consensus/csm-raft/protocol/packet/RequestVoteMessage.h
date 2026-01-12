//
// Created by chu on 4/9/25.
//

#ifndef REQUESTVOTEMSG_H
#define REQUESTVOTEMSG_H

#include "csm-framework/cluster/Common.h"
#include "MessageBase.h"
#include "protocol/pb/RequestVote.pb.h"

namespace csm
{

    namespace consensus
    {

        class RequestVoteMessage : public MessageBase
        {
        public:
            using Ptr = std::shared_ptr<RequestVoteMessage>;

            RequestVoteMessage();
            ~RequestVoteMessage() override = default;

        public:
            void setTerm(std::uint64_t term);
            std::uint64_t term() const;

            void setCandidateId(const NodeId& nodeId);
            NodeId candidateId() const;

            void setLastLogIndex(const std::uint64_t index);
            std::uint64_t lastLogIndex() const;

            void setLastLogTerm(const std::uint64_t term);
            std::uint64_t lastLogTerm() const;

            std::size_t encodeSize() const override;
            void encode(char* buffer, std::size_t size) const override;
            void decode(const char* buffer, std::size_t size) override;

        private:
            RawRequestVote m_rawRequestVote;
        };

    }

}



#endif //REQUESTVOTE_H
