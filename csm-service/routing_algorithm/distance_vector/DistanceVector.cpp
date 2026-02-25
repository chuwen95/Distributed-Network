//
// Created by ivy on 1/8/26.
//

#include "DistanceVector.h"

#include <algorithm>
#include <google/protobuf/message.h>

#include "csm-utilities/Logger.h"

using namespace csm::service;

DistanceVector::DistanceVector(const NodeIds& nodeIds)
{
    std::ranges::for_each(nodeIds, [this](const NodeId& nodeId)
    {
        m_dvInfos[nodeId].distance = c_unreachableDistance;
        m_dvInfos[nodeId].nextHop = nodeId;

        m_neighboursDVInfo[nodeId] = std::unordered_map<NodeId, std::uint32_t>();
    });
}

csm::NodeIds DistanceVector::neighbours() const
{
    NodeIds nodeIds;
    for (const auto& [nodeId, dvInfo] : m_neighboursDVInfo)
    {
        nodeIds.push_back(nodeId);
    }

    return nodeIds;
}

bool DistanceVector::updateNeighbourDistance(const NodeId& peerNodeId, std::uint32_t distance)
{
    bool isUpdated{false};

    std::unique_lock<std::mutex> ulock(x_dvInfos);

    auto iter = m_dvInfos.find(peerNodeId);
    if (m_dvInfos.end() != iter)
    {
        if (iter->second.nextHop == peerNodeId)
        {
            if (iter->second.distance != distance)
            {
                iter->second.distance = distance;
                isUpdated = true;
            }
        }
        else
        {
            if (distance < iter->second.distance)
            {
                iter->second.distance = distance;
                iter->second.nextHop = peerNodeId;

                isUpdated = true;
            }
        }
    }
    else
    {
        LOG->write(utilities::LogType::Log_Error, FILE_INFO, "node not found in neighbour map");
    }

    std::ranges::for_each(m_dvInfos, [](const auto& dvInfo)
    {
        LOG->write(utilities::LogType::Log_Info, FILE_INFO, "node id: ", dvInfo.first, ", distance: ",
                   dvInfo.second.distance);
    });

    return isUpdated;
}

bool DistanceVector::updateDvInfos(const NodeId& peerNodeId,
                                   const std::vector<std::pair<csm::NodeId, std::uint32_t>>& peerDvInfos)
{
    std::unique_lock<std::mutex> ulock(x_dvInfos);

    auto iter = m_neighboursDVInfo.find(peerNodeId);
    if (m_neighboursDVInfo.end() == iter)
    {
        // 邻居间才互相更新距离矢量
        LOG->write(utilities::LogType::Log_Error, peerNodeId, " is not my neighbour");
        return false;
    }

    if (m_dvInfos.end() == m_dvInfos.find(peerNodeId))
    {
        LOG->write(utilities::LogType::Log_Error, peerNodeId, " not found distance info");
        return false;
    }

    // 刷新邻居距离向量缓存
    iter->second.clear();
    for (const auto& peerDvInfo : peerDvInfos)
    {
        iter->second.emplace(peerDvInfo.first, peerDvInfo.second);
    }

    // 根据邻居传过来的距离向量计算更新自己的距离向量表
    std::uint32_t peerNodeDistance = m_dvInfos[peerNodeId].distance;
    for (const auto& peerDvInfo : peerDvInfos)
    {
        std::uint32_t distance{0};
        if (false == queryNodeDistanceWithoutLock(peerDvInfo.first, distance))
        {
            LOG->write(utilities::LogType::Log_Info, FILE_INFO, "not found distance info");

            m_dvInfos[peerDvInfo.first].distance = peerNodeDistance + peerDvInfo.second;
            m_dvInfos[peerDvInfo.first].nextHop = peerNodeId;
        }
        else
        {
            // 如果distance(本节点 -> peerNodeId) + distance(peerNodeId, peerDvInfo.first) < distance(本节点, peerDvInfo.first)
            if (peerNodeDistance + peerDvInfo.second < distance)
            {
                m_dvInfos[peerDvInfo.first].distance = peerNodeDistance + peerDvInfo.second;
                m_dvInfos[peerDvInfo.first].nextHop = peerNodeId;
            }
        }
    }

    return true;
}

std::vector<std::pair<csm::NodeId, std::uint32_t>> DistanceVector::dvInfo(const csm::NodeId& peerNodeId) const
{
    std::vector<std::pair<csm::NodeId, std::uint32_t>> dvInfos;

    for (const auto& [nodeId, dvInfo] : m_dvInfos)
    {
        // 不需要告知自身到peerNodeId的距离信息，因为peerNodeId不需要计算自己到自己的距离信息
        if (nodeId == peerNodeId)
        {
            continue;
        }

        /*
         * 如果本节点到某个节点（以A指代）的下一跳是B，但是B到A的距离是不可达，则本节点告诉B，我到A也是不可达
         */
        if (dvInfo.nextHop == peerNodeId)
        {
            auto outIter = m_neighboursDVInfo.find(peerNodeId);
            if (m_neighboursDVInfo.end() != outIter)
            {
                auto inIter = outIter->second.find(nodeId);
                if (outIter->second.end() != inIter && inIter->second >= c_unreachableDistance)
                {
                    dvInfos.emplace_back(nodeId, c_unreachableDistance);
                }
                else
                {
                    dvInfos.emplace_back(nodeId, dvInfo.distance);
                }
            }
            else
            {
                dvInfos.emplace_back(nodeId, dvInfo.distance);
            }
        }
        else
        {
            dvInfos.emplace_back(nodeId, dvInfo.distance);
        }
    }

    return dvInfos;
}

std::vector<std::tuple<csm::NodeId, std::uint32_t, csm::NodeId>> DistanceVector::dvInfos() const
{
    std::vector<std::tuple<csm::NodeId, std::uint32_t, csm::NodeId>> dvInfos;
    for (const auto& [nodeId, dvInfo] : m_dvInfos)
    {
        dvInfos.emplace_back(nodeId, dvInfo.distance, dvInfo.nextHop);
    }

    return dvInfos;
}

bool DistanceVector::queryNodeDistanceWithoutLock(const NodeId& nodeId, std::uint32_t& distance)
{
    if (m_dvInfos.end() != m_dvInfos.find(nodeId))
    {
        distance = m_dvInfos[nodeId].distance;
        return true;
    }

    return false;
}
