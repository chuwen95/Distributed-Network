//
// Created by ChuWen on 9/11/23.
//

#include "PayloadHeartBeat.h"

using namespace csm::service;

void PayloadHeartBeat::setNodeId(const std::string &nodeId)
{
    m_protoPacket.set_nodeid(nodeId);
}

std::string PayloadHeartBeat::nodeId() const
{
    return m_protoPacket.nodeid();
}

void PayloadHeartBeat::setSeq(const std::uint64_t seq)
{
    m_protoPacket.set_seq(seq);
}

std::uint64_t PayloadHeartBeat::seq() const
{
    return m_protoPacket.seq();
}

void PayloadHeartBeat::setTimestamp(const std::uint32_t timestamp)
{
    m_protoPacket.set_timestamp(timestamp);
}

std::uint32_t PayloadHeartBeat::timestamp() const
{
    return m_protoPacket.timestamp();
}