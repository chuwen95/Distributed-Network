//
// Created by ChuWen on 9/11/23.
//

#include "PayloadHeartBeat.h"

using namespace csm::service;

std::size_t PayloadHeartBeat::packetLength() const
{
    return m_protoHeartBeat.ByteSizeLong();
}

int PayloadHeartBeat::setId(const std::string &id)
{
    m_protoHeartBeat.set_id(id);

    return 0;
}

std::string PayloadHeartBeat::getId()
{
    return m_protoHeartBeat.id();
}

int PayloadHeartBeat::setTimestamp(const std::uint32_t timestamp)
{
    m_protoHeartBeat.set_timestamp(timestamp);

    return 0;
}

std::uint32_t PayloadHeartBeat::getTimestamp()
{
    return m_protoHeartBeat.timestamp();
}

int PayloadHeartBeat::encode(char *buffer, const std::size_t length) const
{
    m_protoHeartBeat.SerializeToArray(buffer, length);

    return 0;
}

int PayloadHeartBeat::decode(const char *buffer, const std::size_t length)
{
    m_protoHeartBeat.ParseFromArray(buffer, length);

    return 0;
}