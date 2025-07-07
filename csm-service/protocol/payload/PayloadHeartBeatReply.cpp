//
// Created by ChuWen on 9/11/23.
//

#include "PayloadHeartBeatReply.h"

using namespace csm::service;

std::size_t PayloadHeartBeatReply::packetLength() const
{
    return m_protoHeartBeatReply.ByteSizeLong();
}

int PayloadHeartBeatReply::setNodeId(const std::string &id)
{
    m_protoHeartBeatReply.set_nodeid(id);
    return 0;
}

std::string PayloadHeartBeatReply::nodeId() const
{
    return m_protoHeartBeatReply.nodeid();
}

int PayloadHeartBeatReply::setSeq(const std::uint64_t seq)
{
    m_protoHeartBeatReply.set_seq(seq);
    return 0;
}

std::uint64_t PayloadHeartBeatReply::seq() const
{
    return m_protoHeartBeatReply.seq();
}

int PayloadHeartBeatReply::setTransferTime(const std::uint32_t transferTime)
{
    m_protoHeartBeatReply.set_transfertime(transferTime);
    return 0;
}

std::uint32_t PayloadHeartBeatReply::transferTime() const
{
    return m_protoHeartBeatReply.transfertime();
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