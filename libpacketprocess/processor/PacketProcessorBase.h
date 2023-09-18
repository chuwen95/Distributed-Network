//
// Created by root on 9/11/23.
//

#ifndef TCPNETWORK_PACKETPROCESSORBASE_H
#define TCPNETWORK_PACKETPROCESSORBASE_H

#include "../packet/PacketBase.h"
#include "../packet/PacketReplyBase.h"

namespace packetprocess
{

    class PacketProcessorBase
    {
    public:
        using Ptr = std::shared_ptr<PacketProcessorBase>;

        PacketProcessorBase() = default;
        virtual ~PacketProcessorBase() = default;

    public:
        virtual int process(PacketBase::Ptr packet, PacketReplyBase::Ptr) = 0;
    };

}

#endif //TCPNETWORK_PACKETPROCESSORBASE_H
