//
// Created by ChuWen on 9/11/23.
//

#include "PayloadHeartBeatReply.h"

using namespace csm::service;

void PayloadHeartBeatReply::setNodeId(const std::string& id)
{
    m_protoPacket.set_nodeid(id);
}

std::string PayloadHeartBeatReply::nodeId() const
{
    return m_protoPacket.nodeid();
}

void PayloadHeartBeatReply::setSeq(const std::uint64_t seq)
{
    m_protoPacket.set_seq(seq);
}

std::uint64_t PayloadHeartBeatReply::seq() const
{
    return m_protoPacket.seq();
}

void PayloadHeartBeatReply::setTransferTime(const std::uint32_t transferTime)
{
    m_protoPacket.set_transfertime(transferTime);
}

std::uint32_t PayloadHeartBeatReply::transferTime() const
{
    return m_protoPacket.transfertime();
}