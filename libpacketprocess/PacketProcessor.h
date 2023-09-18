//
// Created by root on 9/9/23.
//

#ifndef TCPNETWORK_PACKETPROCESSOR_H
#define TCPNETWORK_PACKETPROCESSOR_H

#include "libcommon/Common.h"

#include "packet/PacketBase.h"
#include "packet/PacketReplyBase.h"

namespace packetprocess
{

    class PacketProcessor
    {
    public:
        PacketProcessor();
        ~PacketProcessor();

    public:
        int init();

        int uninit();

        int start();

        int stop();

        int process(const PacketType packetType, PacketBase::Ptr packet, PacketReplyBase::Ptr replyPacket);
    };

}


#endif //TCPNETWORK_PACKETPROCESSOR_H
