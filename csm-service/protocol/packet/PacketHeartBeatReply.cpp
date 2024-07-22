//
// Created by root on 9/11/23.
//

#include "PacketHeartBeatReply.h"

using namespace csm::service;

std::size_t PacketHeartBeatReply::packetLength() const
{
    return m_protoHeartBeatReply.ByteSizeLong();
}

int PacketHeartBeatReply::setId(const std::string &id)
{
    m_protoHeartBeatReply.set_id(id);

    return 0;
}

std::string PacketHeartBeatReply::getId()
{
    return m_protoHeartBeatReply.id();
}

int PacketHeartBeatReply::setSendTimestamp(const std::uint32_t timestamp)
{
    m_protoHeartBeatReply.set_sendtimestamp(timestamp);

    return 0;
}

std::uint32_t PacketHeartBeatReply::getSendTimestamp()
{
    return m_protoHeartBeatReply.sendtimestamp();
}

int PacketHeartBeatReply::setRecvTimestamp(const std::uint32_t timestamp)
{
    m_protoHeartBeatReply.set_recvtimestamp(timestamp);

    return 0;
}

std::uint32_t PacketHeartBeatReply::getRecvTimestamp()
{
    return m_protoHeartBeatReply.recvtimestamp();
}

int PacketHeartBeatReply::encode(char *buffer, const std::size_t length) const
{
    m_protoHeartBeatReply.SerializeToArray(buffer, length);

    return 0;
}

int PacketHeartBeatReply::decode(const char *buffer, const std::size_t length)
{
    m_protoHeartBeatReply.ParseFromArray(buffer, length);

    return 0;
}