//
// Created by chu on 4/19/25.
//

#ifndef REQUESTVOTEMESSAGEREPLY_H
#define REQUESTVOTEMESSAGEREPLY_H

#include "MessageBase.h"

namespace csm
{

    namespace consensus
    {

        class RequestVoteMessageReply : public MessageBase
        {
        public:
            RequestVoteMessageReply();
            ~RequestVoteMessageReply() override = default;
        };

    }

}

#endif //REQUESTVOTEMESSAGEREPLY_H
