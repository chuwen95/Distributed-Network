//
// Created by ChuWen on 2024/8/23.
//

#ifndef COPYSTATEMACHINE_PAYLOADBASE_H
#define COPYSTATEMACHINE_PAYLOADBASE_H

#include "csm-common/Common.h"

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

    }

}

#endif //COPYSTATEMACHINE_PAYLOADBASE_H
