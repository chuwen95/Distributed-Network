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
            /**
             * 不必担心data被修改，data的数据将被复制
             * @param data
             * @param len
             */
            void setPayload(const char* data, const std::size_t len);
            void setPayload(const std::vector<char>& data);

            std::shared_ptr<std::vector<char>> payload();

        private:
            std::shared_ptr<std::vector<char>> m_payload;
        };

    }

}


#endif //COPYSTATEMACHINE_PAYLOADMODULEMESSAGE_H
