//
// Created by root on 9/11/23.
//

#ifndef TCPNETWORK_PACKETPROCESSORFACTORY_H
#define TCPNETWORK_PACKETPROCESSORFACTORY_H

#include "PacketProcessorBase.h"

namespace packetprocess
{

    class PacketProcessorFactory
    {
    public:
        PacketProcessorFactory() = default;
        ~PacketProcessorFactory() = default;

        PacketProcessorBase::Ptr createPacketProcessor(PacketType type);
    };

} // packetprocess

#endif //TCPNETWORK_PACKETPROCESSORFACTORY_H
