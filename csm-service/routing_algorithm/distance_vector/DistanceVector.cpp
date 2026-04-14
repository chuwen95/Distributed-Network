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
        std::shared_ptr<NodeInfo> nodeInfo = std::make_shared<NodeInfo>(nodeId, c_unreachableDistance);
        m_dvInfos.emplace(nodeId, nodeInfo);

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
        if (iter->second->nextHop == peerNodeId)
        {
            if (iter->second->distance != distance)
            {
                iter->second->distance = (distance > c_unreachableDistance ? c_unreachableDistance : distance);
                if (c_unreachableDistance == iter->second->distance)
                {
                    iter->second->nextHop = csm::c_invalidNodeId;
                }

                isUpdated = true;
            }
        }
        else
        {
            if (distance < iter->second->distance)
            {
                iter->second->distance = distance;
                iter->second->nextHop = peerNodeId;

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
                   dvInfo.second->distance);
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
    std::uint32_t peerNodeDistance = m_dvInfos[peerNodeId]->distance;
    for (const auto& peerDvInfo : peerDvInfos)
    {
        std::shared_ptr<NodeInfo> nodeInfo = queryNodeInfoWithoutLock(peerDvInfo.first);
        if (nullptr == nodeInfo)
        {
            LOG->write(utilities::LogType::Log_Info, FILE_INFO, "not found distance info");

            std::shared_ptr<NodeInfo> newNodeInfo = std::make_shared<NodeInfo>(
                peerNodeId, peerNodeDistance + peerDvInfo.second);
            m_dvInfos.emplace(peerDvInfo.first, newNodeInfo);
        }
        else
        {
            if (nodeInfo->nextHop == peerNodeId)
            {
                nodeInfo->distance = peerNodeDistance + peerDvInfo.second;
                if (nodeInfo->distance > csm::service::c_unreachableDistance)
                {
                    nodeInfo->nextHop = csm::c_invalidNodeId;
                    nodeInfo->distance = csm::service::c_unreachableDistance;
                }
            }
            else
            {
                // 如果distance(本节点 -> peerNodeId) + distance(peerNodeId, peerDvInfo.first) < distance(本节点, peerDvInfo.first)
                if (peerNodeDistance + peerDvInfo.second < nodeInfo->distance)
                {
                    m_dvInfos[peerDvInfo.first]->distance = peerNodeDistance + peerDvInfo.second;
                    m_dvInfos[peerDvInfo.first]->nextHop = peerNodeId;
                }
            }

        }
    }

    return true;
}

std::vector<std::pair<csm::NodeId, std::uint32_t>> DistanceVector::dvInfo(const csm::NodeId& peerNodeId)
{
    std::unique_lock<std::mutex> ulock(x_dvInfos);

    std::vector<std::pair<csm::NodeId, std::uint32_t>> dvInfos;

    for (const auto& [nodeId, dvInfo] : m_dvInfos)
    {
        // 不需要告知自身到peerNodeId的距离信息，因为peerNodeId不需要计算自己到自己的距离信息
        if (nodeId == peerNodeId)
        {
            continue;
        }

        /*
         * 如果本节点到某个节点（以A指代）的下一跳是B，证明本节点到A的路径是从B这里学到的
         * 根据毒性逆转规则，本节点告诉B到：我到A不可达
         */
        if (dvInfo->nextHop == peerNodeId)
        {
            dvInfos.emplace_back(nodeId, c_unreachableDistance);
        }
        else
        {
            dvInfos.emplace_back(nodeId, dvInfo->distance);
        }
    }

    return dvInfos;
}

std::vector<std::tuple<csm::NodeId, std::uint32_t, csm::NodeId>> DistanceVector::dvInfos()
{
    std::unique_lock<std::mutex> ulock(x_dvInfos);

    std::vector<std::tuple<csm::NodeId, std::uint32_t, csm::NodeId>> dvInfos;
    for (const auto& [nodeId, dvInfo] : m_dvInfos)
    {
        dvInfos.emplace_back(nodeId, dvInfo->distance, dvInfo->nextHop);
    }

    return dvInfos;
}

auto DistanceVector::queryNodeInfoWithoutLock(const NodeId& nodeId) -> std::shared_ptr<NodeInfo>
{
    auto iter = m_dvInfos.find(nodeId);
    if (m_dvInfos.end() != iter)
    {
        return iter->second;
    }

    return nullptr;
}

std::optional<std::pair<std::uint32_t, csm::NodeId>> DistanceVector::distance(const NodeId& target)
{
    std::unique_lock<std::mutex> ulock(x_dvInfos);

    auto iter = m_dvInfos.find(target);
    if (m_dvInfos.end() == iter)
    {
        return std::nullopt;
    }

    return std::make_pair(iter->second->distance, iter->second->nextHop);
}
