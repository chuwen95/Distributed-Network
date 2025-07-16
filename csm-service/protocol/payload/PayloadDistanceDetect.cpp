//
// Created by chu on 7/9/25.
//

#include "PayloadDistanceDetect.h"

using namespace csm::service;

void PayloadDistanceDetect::setSeq(std::uint32_t seq)
{
    m_protoPacket.set_seq(seq);
}

std::uint32_t PayloadDistanceDetect::seq() const
{
    return m_protoPacket.seq();
}

void PayloadDistanceDetect::setTimestamp(std::uint32_t timestamp)
{
    m_protoPacket.set_timestamp(timestamp);
}

std::uint32_t PayloadDistanceDetect::timestamp() const
{
    return m_protoPacket.timestamp();
}