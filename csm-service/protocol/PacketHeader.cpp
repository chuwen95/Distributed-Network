//
// Created by root on 9/11/23.
//

#include "PacketHeader.h"

using namespace csm::service;

bool PacketHeader::isMagicMatch()
{
    return c_magic == m_magic;
}

int PacketHeader::setType(PacketType type)
{
    m_packetType = static_cast<std::uint32_t>(type);

    return 0;
}

PacketType PacketHeader::type() const
{
    return static_cast<PacketType>(m_packetType);
}

int PacketHeader::setPayloadLength(std::uint32_t payloadLength)
{
    m_payloadLength = payloadLength;

    return 0;
}

std::uint32_t PacketHeader::payloadLength() const
{
    return m_payloadLength;
}

int PacketHeader::setModuleId(const std::int32_t moduleId)
{
    m_moduleId = moduleId;

    return 0;
}

std::int32_t PacketHeader::moduleId() const
{
    return m_moduleId;
}

std::size_t PacketHeader::headerLength()
{
    return sizeof(m_magic) + sizeof(m_packetType) + sizeof(m_moduleId) + sizeof(m_payloadLength);
}

int PacketHeader::encode(char *buffer, const std::size_t length)
{
    if (length != headerLength())
    {
        return -1;
    }

    *(reinterpret_cast<std::uint32_t *>(buffer)) = htonl(m_magic);
    *(reinterpret_cast<std::uint16_t *>(buffer + sizeof(m_magic))) = htons(m_packetType);
    *(reinterpret_cast<std::int32_t *>(buffer + sizeof(m_magic) + sizeof(m_packetType))) = htonl(m_moduleId);
    *(reinterpret_cast<std::uint32_t *>(buffer + sizeof(m_magic) + sizeof(m_packetType) + sizeof(m_moduleId))) = htonl(
            m_payloadLength);

    return 0;
}

int PacketHeader::decode(const char *buffer, const std::size_t length)
{
    if (length < headerLength())
    {
        return -1;
    }

    m_magic = ntohl(*(reinterpret_cast<std::uint32_t *>(const_cast<char *>(buffer))));
    m_packetType = ntohs(*(reinterpret_cast<std::uint16_t *>(const_cast<char *>(buffer + sizeof(m_magic)))));
    m_moduleId = ntohl(
            *(reinterpret_cast<std::int32_t *>(const_cast<char *>(buffer + sizeof(m_magic) + sizeof(m_packetType)))));
    m_payloadLength = ntohl(
            *(reinterpret_cast<std::uint32_t *>(const_cast<char *>(buffer + sizeof(m_magic) + sizeof(m_packetType) +
                                                                   sizeof(m_moduleId)))));

    return 0;
}