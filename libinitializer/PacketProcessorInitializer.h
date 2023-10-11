//
// Created by root on 10/11/23.
//

#ifndef TCPNETWORK_PACKETPROCESSORINITIALIZER_H
#define TCPNETWORK_PACKETPROCESSORINITIALIZER_H

#include "libpacketprocess/PacketProcessor.h"

namespace initializer
{

    class PacketProcessorInitializer
    {
    public:
        using Ptr = std::shared_ptr<PacketProcessorInitializer>;

        PacketProcessorInitializer();
        ~PacketProcessorInitializer() = default;

    public:
        int init();

        int uninit();

        packetprocess::PacketProcessor::Ptr  packetProcessor();

    private:
        packetprocess::PacketProcessor::Ptr m_packetProcessor;
    };

} // initializer

#endif //TCPNETWORK_PACKETPROCESSORINITIALIZER_H
