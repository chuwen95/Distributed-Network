//
// Created by chu on 4/13/25.
//

#ifndef MESSAGEBASE_H
#define MESSAGEBASE_H

#include <protocol/pb/RaftMessage.pb.h>

#include "csm-common/Common.h"

namespace csm
{

    namespace consensus
    {

        class MessageBase
        {
        public:
            using Ptr = std::shared_ptr<MessageBase>;

            MessageBase(const RaftMessageType type) : m_type(type) {}
            virtual ~MessageBase() = default;

        public:
            virtual std::size_t encodeSize() const = 0;

            virtual void encode(char* buffer, std::size_t size) const = 0;
            virtual void decode(const char* buffer, std::size_t size) = 0;

        protected:
            RaftMessageType m_type;
        };

    }

}

#endif //MESSAGEBASE_H
