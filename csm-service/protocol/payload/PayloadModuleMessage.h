//
// Created by ChuWen on 2024/8/24.
//

#ifndef COPYSTATEMACHINE_PAYLOADMODULEMESSAGE_H
#define COPYSTATEMACHINE_PAYLOADMODULEMESSAGE_H

#include "csm-common/Common.h"
#include "PayloadBase.h"

namespace csm
{

    namespace service
    {

        class PayloadModuleMessage : public PayloadBase
        {
        public:
            using Ptr = std::shared_ptr<PayloadModuleMessage>;

            PayloadModuleMessage();
            ~PayloadModuleMessage() = default;

        public:
            std::size_t packetLength() const override;

            int encode(char *buffer, const std::size_t length) const override;
            int decode(const char *buffer, const std::size_t length) override;

            std::shared_ptr<std::vector<char>> payload();

        private:
            std::shared_ptr<std::vector<char>> m_payload;
        };

    }

}


#endif //COPYSTATEMACHINE_PAYLOADMODULEMESSAGE_H
