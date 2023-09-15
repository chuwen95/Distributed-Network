//
// Created by root on 9/11/23.
//

#ifndef TCPNETWORK_PACKETHEADER_H
#define TCPNETWORK_PACKETHEADER_H

#include "libcommon/Common.h"

namespace packetprocess
{

    constexpr std::uint32_t c_magic{0x2cc98f5d};

    enum class PacketType
    {
        PT_None,

        PT_HeartBeat,
        PT_HeartBeatReply,
        PT_RawString,
        PT_RawStringReply
    };

    class PacketHeader
    {
    public:
        using Ptr = std::shared_ptr<PacketHeader>;

        PacketHeader() = default;
        ~PacketHeader() = default;

    public:
        bool isMagicMatch();

        int setType(PacketType type);
        PacketType type();

        int setPayloadLength(std::size_t payloadLength);
        std::size_t payloadLength();

        std::size_t headerLength();

        int encode(char* buffer, const std::size_t length);
        int decode(const char* buffer, const std::size_t length);

    private:
        std::uint32_t m_magic{c_magic};
        std::uint16_t m_packetType;
        std::uint32_t m_payloadLength;
    };

} // packetprocess

#endif //TCPNETWORK_PACKETHEADER_H
