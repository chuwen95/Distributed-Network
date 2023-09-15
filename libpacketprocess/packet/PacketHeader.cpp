//
// Created by root on 9/11/23.
//

#include "PacketHeader.h"

namespace packetprocess
{

    bool PacketHeader::isMagicMatch()
    {
        return c_magic == m_magic;
    }

    int PacketHeader::setType(packetprocess::PacketType type)
    {
        m_packetType = static_cast<std::uint32_t>(type);

        return 0;
    }

    PacketType PacketHeader::type()
    {
        return static_cast<PacketType>(m_packetType);
    }

    int PacketHeader::setPayloadLength(std::size_t payloadLength)
    {
        m_payloadLength = payloadLength;

        return 0;
    }

    std::size_t PacketHeader::payloadLength()
    {
        return m_payloadLength;
    }

    std::size_t PacketHeader::headerLength()
    {
        return sizeof(m_magic) + sizeof(m_packetType) + sizeof(m_payloadLength);
    }

    int PacketHeader::encode(char *buffer, const std::size_t length)
    {
        if(length != headerLength())
        {
            return -1;
        }

        *(reinterpret_cast<std::uint32_t*>(buffer)) = htonl(m_magic);
        *(reinterpret_cast<std::uint16_t*>(buffer + sizeof(m_magic))) = htons(m_packetType);
        *(reinterpret_cast<std::uint32_t*>(buffer + sizeof(m_magic) + sizeof(m_packetType))) = htonl(m_payloadLength);

        return 0;
    }

    int PacketHeader::decode(const char *buffer, const std::size_t length)
    {
        if(length < headerLength())
        {
            return -1;
        }

        m_magic = ntohl(*(reinterpret_cast<std::uint32_t*>(const_cast<char*>(buffer))));
        m_packetType = ntohs(*(reinterpret_cast<std::uint16_t*>(const_cast<char*>(buffer + sizeof(m_magic)))));
        m_payloadLength = ntohl(*(reinterpret_cast<std::uint32_t*>(const_cast<char*>(buffer + sizeof(m_magic) + sizeof(m_packetType)))));

        return 0;
    }

} // packetprocess