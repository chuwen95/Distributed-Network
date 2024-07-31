//
// Created by ChuWen on 9/11/23.
//

#include "PacketHeartBeat.h"

using namespace csm::service;

std::size_t PacketHeartBeat::packetLength() const
{
    return m_protoHeartBeat.ByteSizeLong();
}

int PacketHeartBeat::setId(const std::string &id)
{
    m_protoHeartBeat.set_id(id);

    return 0;
}

std::string PacketHeartBeat::getId()
{
    return m_protoHeartBeat.id();
}

int PacketHeartBeat::setTimestamp(const std::uint32_t timestamp)
{
    m_protoHeartBeat.set_timestamp(timestamp);

    return 0;
}

std::uint32_t PacketHeartBeat::getTimestamp()
{
    return m_protoHeartBeat.timestamp();
}

int PacketHeartBeat::encode(char *buffer, const std::size_t length) const
{
    m_protoHeartBeat.SerializeToArray(buffer, length);

    return 0;
}

int PacketHeartBeat::decode(const char *buffer, const std::size_t length)
{
    m_protoHeartBeat.ParseFromArray(buffer, length);

    return 0;
}