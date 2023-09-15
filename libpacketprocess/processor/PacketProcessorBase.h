//
// Created by root on 9/11/23.
//

#ifndef TCPNETWORK_PACKETPROCESSORBASE_H
#define TCPNETWORK_PACKETPROCESSORBASE_H

#include "../packet/PacketBase.h"

namespace packetprocess
{

    class PacketProcessorBase
    {
    public:
        using Ptr = std::shared_ptr<PacketProcessorBase>;

        PacketProcessorBase() = default;
        virtual ~PacketProcessorBase() = default;

    public:
        virtual int process(const int fd, PacketBase::Ptr packet, const std::function<int(const int, const std::vector<char>&)>& writeHandler) = 0;
    };

}

#endif //TCPNETWORK_PACKETPROCESSORBASE_H
