//
// Created by ChuWen on 2024/8/23.
//

#ifndef COPYSTATEMACHINE_PAYLOADFACTORY_H
#define COPYSTATEMACHINE_PAYLOADFACTORY_H

#include "csm-service/protocol/common/PacketType.h"
#include "PayloadBase.h"

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
            PayloadBase::Ptr createPayload(PacketType packetType);
            PayloadBase::Ptr createPayload( PacketType packetType, const char* data, std::size_t len);
        };

    }

}

#endif //COPYSTATEMACHINE_PAYLOADFACTORY_H
