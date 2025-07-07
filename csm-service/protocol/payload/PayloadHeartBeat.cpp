//
// Created by ChuWen on 9/11/23.
//

#include "PayloadHeartBeat.h"

using namespace csm::service;

std::size_t PayloadHeartBeat::packetLength() const
{
    return m_protoHeartBeat.ByteSizeLong();
}

int PayloadHeartBeat::setNodeId(const std::string &nodeId)
{
    m_protoHeartBeat.set_nodeid(nodeId);
    return 0;
}

std::string PayloadHeartBeat::nodeId() const
{
    return m_protoHeartBeat.nodeid();
}

int PayloadHeartBeat::setSeq(const std::uint64_t seq)
{
    m_protoHeartBeat.set_seq(seq);
    return 0;
}

std::uint64_t PayloadHeartBeat::seq() const
{
    return m_protoHeartBeat.seq();
}

int PayloadHeartBeat::setTimestamp(const std::uint32_t timestamp)
{
    m_protoHeartBeat.set_timestamp(timestamp);
    return 0;
}

std::uint32_t PayloadHeartBeat::timestamp() const
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