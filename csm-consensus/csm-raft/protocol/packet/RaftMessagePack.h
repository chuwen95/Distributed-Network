//
// Created by chu on 4/9/25.
//

#ifndef RAFTMESSAGE_H
#define RAFTMESSAGE_H

#include "csm-common/Common.h"

#include "protocol/pb/RaftMessage.pb.h"

namespace csm
{

    namespace consensus
    {

        class RaftMessagePack
        {
        public:
            using Ptr = std::shared_ptr<RaftMessagePack>;

            RaftMessagePack() = default;
            ~RaftMessagePack() = default;

        public:
            void setMessageType(RaftMessageType type);
            RaftMessageType messageType() const;

            void setNodeIndex(const std::uint32_t nodeIndex);
            std::uint32_t nodeIndex() const;

            void setPayloadSize(size_t size);
            std::size_t payloadSize() const;
            char* payloadRawPointer();

            std::size_t encodeSize() const;
            void encode(char* buffer, std::size_t size) const;
            void decode(char* buffer, std::size_t size);

        private:
            RawRaftMessage m_rawRaftMessage;
        };

    }

}

#endif //RAFTMESSAGE_H
