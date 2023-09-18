//
// Created by root on 9/18/23.
//

#ifndef TCPNETWORK_PACKETREPLYBASE_H
#define TCPNETWORK_PACKETREPLYBASE_H

#include "libcommon/Common.h"
#include "PacketHeader.h"

namespace packetprocess
{

    class PacketReplyBase
    {
    public:
        using Ptr = std::shared_ptr<PacketReplyBase>;

        PacketReplyBase() = default;
        virtual ~PacketReplyBase() = default;

    public:
        virtual std::size_t packetLength() = 0;

        virtual int encode(char* buffer, const std::size_t length) = 0;
        virtual int decode(const char* buffer, const std::size_t length) = 0;
    };

} // packetprocess

#endif //TCPNETWORK_PACKETREPLYBASE_H
