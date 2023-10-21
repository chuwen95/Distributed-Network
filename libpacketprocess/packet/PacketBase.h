//
// Created by root on 9/11/23.
//

#ifndef TCPNETWORK_PACKETBASE_H
#define TCPNETWORK_PACKETBASE_H

#include "libcommon/Common.h"
#include "PacketHeader.h"

namespace packetprocess
{

    class PacketBase
    {
    public:
        using Ptr = std::shared_ptr<PacketBase>;

        PacketBase() = default;
        virtual ~PacketBase() = default;

    public:
        virtual std::size_t packetLength() const = 0;

        virtual int encode(char* buffer, const std::size_t length) const = 0;
        virtual int decode(const char* buffer, const std::size_t length) = 0;
    };

} // packetprocess

#endif //TCPNETWORK_PACKETBASE_H
