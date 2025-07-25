//
// Created by chu on 7/18/25.
//

#include "PayloadDistanceVector.h"

using namespace csm::service;

std::vector<std::pair<csm::NodeId, std::uint32_t>> PayloadDistanceVector::distanceInfos() const
{
    std::vector<std::pair<csm::NodeId, std::uint32_t>> distanceInfos;

    for (const protocol::DistanceInfo& info : m_protoPacket.distancevector())
    {
        distanceInfos.emplace_back(info.nodeid(), info.distance());
    }

    return distanceInfos;
}

void PayloadDistanceVector::addDistanceInfo(const csm::NodeId& key, const std::uint32_t value)
{
    protocol::DistanceInfo* distanceInfo = m_protoPacket.add_distancevector();
    distanceInfo->set_nodeid(key);
    distanceInfo->set_distance(value);
}