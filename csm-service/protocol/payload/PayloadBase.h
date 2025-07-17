//
// Created by ChuWen on 2024/8/23.
//

#ifndef COPYSTATEMACHINE_PAYLOADBASE_H
#define COPYSTATEMACHINE_PAYLOADBASE_H

#include <memory>

#include "csm-service/protocol/common/PacketType.h"

namespace csm
{

    namespace service
    {

        class PayloadBase
        {
        public:
            using Ptr = std::shared_ptr<PayloadBase>;

            PayloadBase() = default;
            virtual ~PayloadBase() = default;

        public:
            virtual std::size_t packetLength() const = 0;
            virtual int encode(char *buffer, const std::size_t length) const = 0;
            virtual int decode(const char *buffer, const std::size_t length) = 0;
        };

        template<typename T>
        class PayloadTypeBase : public PayloadBase
        {
        public:
            std::size_t packetLength() const override
            {
                return m_protoPacket.ByteSizeLong();
            }

            int encode(char *buffer, const std::size_t length) const override
            {
                return true == m_protoPacket.SerializeToArray(buffer, length) ? 0 : -1;
            }

            int decode(const char *buffer, const std::size_t length) override
            {
                return true == m_protoPacket.ParseFromArray(buffer, length) ? 0 : -1;
            }

        protected:
            T m_protoPacket;
        };
    }

}

#endif //COPYSTATEMACHINE_PAYLOADBASE_H
