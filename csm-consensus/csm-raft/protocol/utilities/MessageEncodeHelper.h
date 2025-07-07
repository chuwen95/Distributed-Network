//
// Created by chu on 4/24/25.
//

#ifndef MESSAGEENCODEHELPER_H
#define MESSAGEENCODEHELPER_H

#include "csm-common/Common.h"
#include "csm-consensus/csm-raft/protocol/packet/MessageBase.h"

namespace csm
{

    namespace consensus
    {

        class MessageEncodeHelper
        {
        public:
            using Ptr = std::shared_ptr<MessageEncodeHelper>;

            MessageEncodeHelper() = default;
            ~MessageEncodeHelper() = default;

        public:
            static int encode(MessageBase::Ptr message, std::vector<char>& buffer);
        };

    }

}



#endif //MESSAGEENCODEHELPER_H
