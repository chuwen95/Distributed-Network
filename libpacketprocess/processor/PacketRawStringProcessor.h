//
// Created by root on 9/11/23.
//

#ifndef TCPNETWORK_PACKETRAWSTRINGPROCESSOR_H
#define TCPNETWORK_PACKETRAWSTRINGPROCESSOR_H

#include "PacketProcessorBase.h"

namespace packetprocess
{

    class PacketRawStringProcessor : public PacketProcessorBase
    {
    public:
        PacketRawStringProcessor() = default;
        ~PacketRawStringProcessor() = default;

        int process(PacketBase::Ptr packet, PacketReplyBase::Ptr) override;
    };

} // packetprocessor

#endif //TCPNETWORK_PACKETRAWSTRINGPROCESSOR_H
