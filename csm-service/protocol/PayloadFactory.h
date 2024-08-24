//
// Created by ChuWen on 2024/8/23.
//

#ifndef COPYSTATEMACHINE_PAYLOADFACTORY_H
#define COPYSTATEMACHINE_PAYLOADFACTORY_H

#include "csm-common/Common.h"
#include "PacketHeader.h"
#include "payload/PayloadBase.h"

namespace csm
{

    namespace service
    {

        class PayloadFactory
        {
        public:
            using Ptr = std::shared_ptr<PayloadFactory>;

            PayloadFactory() = default;
            ~PayloadFactory() = default;

        public:
            PayloadBase::Ptr createPayload(const PacketType packetType);
            PayloadBase::Ptr createPayload(const PacketType packetType, const char* data, const std::size_t len);
        };

    }

}

#endif //COPYSTATEMACHINE_PAYLOADFACTORY_H
