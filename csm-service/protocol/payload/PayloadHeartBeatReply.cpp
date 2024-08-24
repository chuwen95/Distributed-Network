//
// Created by ChuWen on 9/11/23.
//

#include "PayloadHeartBeatReply.h"

using namespace csm::service;

std::size_t PayloadHeartBeatReply::packetLength() const
{
    return m_protoHeartBeatReply.ByteSizeLong();
}

int PayloadHeartBeatReply::setId(const std::string &id)
{
    m_protoHeartBeatReply.set_id(id);

    return 0;
}

std::string PayloadHeartBeatReply::getId()
{
    return m_protoHeartBeatReply.id();
}

int PayloadHeartBeatReply::setSendTimestamp(const std::uint32_t timestamp)
{
    m_protoHeartBeatReply.set_sendtimestamp(timestamp);

    return 0;
}

std::uint32_t PayloadHeartBeatReply::getSendTimestamp()
{
    return m_protoHeartBeatReply.sendtimestamp();
}

int PayloadHeartBeatReply::setRecvTimestamp(const std::uint32_t timestamp)
{
    m_protoHeartBeatReply.set_recvtimestamp(timestamp);

    return 0;
}

std::uint32_t PayloadHeartBeatReply::getRecvTimestamp()
{
    return m_protoHeartBeatReply.recvtimestamp();
}

int PayloadHeartBeatReply::encode(char *buffer, const std::size_t length) const
{
    m_protoHeartBeatReply.SerializeToArray(buffer, length);

    return 0;
}

int PayloadHeartBeatReply::decode(const char *buffer, const std::size_t length)
{
    m_protoHeartBeatReply.ParseFromArray(buffer, length);

    return 0;
}