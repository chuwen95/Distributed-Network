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

        int process(const int fd, PacketBase::Ptr packet, const std::function<int(const int, const std::vector<char>&)>& writeHandler) override;
    };

} // packetprocessor

#endif //TCPNETWORK_PACKETRAWSTRINGPROCESSOR_H
