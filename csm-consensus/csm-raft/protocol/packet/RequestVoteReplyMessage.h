//
// Created by chu on 4/19/25.
//

#ifndef REQUESTVOTEREPLYMESSAGE_H
#define REQUESTVOTEREPLYMESSAGE_H

#include "MessageBase.h"
#include "protocol/pb/RequestVote.pb.h"

namespace csm
{

    namespace consensus
    {

        class RequestVoteReplyMessage : public MessageBase
        {
        public:
            using Ptr = std::shared_ptr<RequestVoteReplyMessage>;

            RequestVoteReplyMessage();
            ~RequestVoteReplyMessage() override = default;

        public:
            void setTerm(const std::uint64_t term);
            std::uint64_t term() const;

            void setVoteGranted(const bool voteGranted);
            bool voteGranted() const;

            std::size_t encodeSize() const override;
            void encode(char* buffer, std::size_t size) const override;
            void decode(const char* buffer, std::size_t size) override;

        private:
            RawRequestVoteReply m_rawRequestVoteReply;
        };

    }

}

#endif //REQUESTVOTEREPLYMESSAGE_H
