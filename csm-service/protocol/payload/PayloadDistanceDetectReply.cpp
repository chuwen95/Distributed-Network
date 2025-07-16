//
// Created by chu on 7/9/25.
//

#include "PayloadDistanceDetectReply.h"

using namespace csm::service;

void PayloadDistanceDetectReply::setSeq(const std::uint64_t seq)
{
    m_protoPacket.set_seq(seq);
}

std::uint64_t PayloadDistanceDetectReply::seq() const
{
    return m_protoPacket.seq();
}

void PayloadDistanceDetectReply::setElapsedTime(const std::uint32_t elapsedTime)
{
    m_protoPacket.set_elapsedtime(elapsedTime);
}

std::uint32_t PayloadDistanceDetectReply::elapsedTime() const
{
    return m_protoPacket.elapsedtime();
}