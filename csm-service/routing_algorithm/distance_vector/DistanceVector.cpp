//
// Created by ivy on 1/8/26.
//

#include "DistanceVector.h"

#include <algorithm>
#include <google/protobuf/message.h>

#include "csm-utilities/Logger.h"

using namespace csm::service;

DistanceVector::DistanceVector(NodeId nodeId, const NodeIds& nodeIds) : m_selfNodeId(std::move(nodeId))
{
    std::ranges::for_each(nodeIds, [this](const NodeId& nodeId)
    {
        m_dvInfos.emplace(nodeId, NodeInfo());
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
    if (peerNodeId == m_selfNodeId || peerNodeId == c_invalidNodeId)
    {
        return false;
    }

    std::unique_lock<std::mutex> ulock(x_dvInfos);

    auto iter = m_dvInfos.find(peerNodeId);
    if (m_dvInfos.end() == iter)
    {
        LOG->write(utilities::LogType::Log_Error, FILE_INFO, "node not found in neighbour map");
        return false;
    }

    bool isDirectChanged{false};
    if (iter->second.nextHop != peerNodeId || iter->second.distance != distance)
    {
        iter->second.nextHop = peerNodeId;
        iter->second.distance = distance;
        if (iter->second.distance >= c_unreachableDistance)
        {
            iter->second.nextHop = c_invalidNodeId;

            // 清除邻居的DV信息
            /*
             * 如果不清除，当邻居不可达时，recomputeRoutesWithoutLock()会跳过它，所以旧缓存不生效
             * 但如果后面到邻居的链路生效了，而对方还没来得及重新发送最新DV，那么邻居的旧DV会立即被使用，
             * 但可能旧DV中的信息已经过时，比如该邻居到某个节点已经不可达
             */
            m_neighboursDVInfo[peerNodeId].clear();
        }

        isDirectChanged = true;
    }

    bool isRecomputed = recomputeRoutesWithoutLock();

    std::ranges::for_each(m_dvInfos, [](const auto& dvInfo)
    {
        LOG->write(utilities::LogType::Log_Info, FILE_INFO, "node id: ", dvInfo.first, ", distance: ",
                   dvInfo.second.distance);
    });

    return isDirectChanged || isRecomputed;
}

bool DistanceVector::updateDvInfos(const NodeId& peerNodeId,
                                   const std::vector<std::pair<csm::NodeId, std::uint32_t>>& peerDvInfos)
{
    if (peerNodeId == m_selfNodeId || peerNodeId == c_invalidNodeId)
    {
        return false;
    }

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
        if (m_selfNodeId == peerNodeId)
        {
            // 这个if本来不会进，但是在一次单元测试中，
            // A --2-- E --1-- B
            // 我写成了:
            // distanceVectorA.updateDvInfos(distanceVectorE.dvInfo("E"));
            // 后来对dvInfo方法也做了限制，不允许获取针对自身的DvInfo
            LOG->write(utilities::LogType::Log_Error, "DvInfos contains this node");
            continue;
        }

        auto singleNeighbourDvInfoIter = iter->second.find(peerDvInfo.first);
        if (iter->second.end() != singleNeighbourDvInfoIter)
        {
            LOG->write(utilities::LogType::Log_Warning, "DvInfos exist duplicate target node: ", peerDvInfo.first, ", ",
                       peerDvInfo.second);
            if (peerDvInfo.second < singleNeighbourDvInfoIter->second)
            {
                LOG->write(utilities::LogType::Log_Warning, "Duplicate dv info distance smaller, use this");
                singleNeighbourDvInfoIter->second = peerDvInfo.second;
            }
        }
        iter->second.emplace(peerDvInfo.first, peerDvInfo.second);
    }

    // 全量重算距离向量表
    return recomputeRoutesWithoutLock();
}

std::vector<std::pair<csm::NodeId, std::uint32_t>> DistanceVector::dvInfo(const csm::NodeId& peerNodeId) const
{
    if (peerNodeId == m_selfNodeId)
    {
        return {};
    }

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
        if (dvInfo.nextHop == peerNodeId)
        {
            dvInfos.emplace_back(nodeId, c_unreachableDistance);
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
    std::unique_lock<std::mutex> ulock(x_dvInfos);

    std::vector<std::tuple<csm::NodeId, std::uint32_t, csm::NodeId>> dvInfos;
    for (const auto& [nodeId, dvInfo] : m_dvInfos)
    {
        dvInfos.emplace_back(nodeId, dvInfo.distance, dvInfo.nextHop);
    }

    return dvInfos;
}

bool DistanceVector::recomputeRoutesWithoutLock()
{
    bool isUpdated{false};

    std::unordered_set<NodeId> targetNodes;
    for (const auto& nodeId : m_dvInfos | std::views::keys)
    {
        targetNodes.insert(nodeId);
    }
    for (const auto& neighbourDvInfo : m_neighboursDVInfo)
    {
        targetNodes.insert(neighbourDvInfo.first);
        for (const auto& nodeId : neighbourDvInfo.second | std::views::keys)
        {
            targetNodes.insert(nodeId);
        }
    }

    for (const NodeId& targetNodeId : targetNodes)
    {
        NodeId bestNextHop{c_invalidNodeId};
        std::uint32_t bestDistance{c_unreachableDistance};

        // 遍历本节点存储的邻居的距离向量
        for (const auto& [neighbourNodeId, neighbourDvInfo] : m_neighboursDVInfo)
        {
            auto neighbourNodeInfoIter = m_dvInfos.find(neighbourNodeId);
            if (m_dvInfos.end() == neighbourNodeInfoIter)
            {
                continue;
            }

            if (neighbourNodeInfoIter->second.distance >= c_unreachableDistance)
            {
                continue;
            }

            std::uint32_t distance;
            if (neighbourNodeId == targetNodeId)
            {
                distance = neighbourNodeInfoIter->second.distance;
            }
            else
            {
                auto targetIter = neighbourDvInfo.find(targetNodeId);
                if (neighbourDvInfo.end() == targetIter)
                {
                    continue;
                }

                distance = addDistance(neighbourNodeInfoIter->second.distance, targetIter->second);
            }

            // 其实当bestDistance > distance就足以触发更新
            // (bestDistance == distance && neighbourNodeId < bestNextHop)是为了避免：
            //     同样的拓扑，同样的代价，但可能出现选出的下一跳不同的情况，从而导致：
            //     1. 行为不可预测
            //     2. 单元测试不好写
            //     3. 后面可能难以定位问题，难以解释：“为什么这次选E，下次选D”
            if (bestDistance > distance || (bestDistance == distance && neighbourNodeId < bestNextHop))
            {
                bestDistance = distance;
                bestNextHop = neighbourNodeId;
                if (bestDistance >= c_unreachableDistance)
                {
                    bestNextHop = c_invalidNodeId;
                }
            }
        }

        auto iter = m_dvInfos.find(targetNodeId);
        if (m_dvInfos.end() == iter)
        {
            m_dvInfos.emplace(targetNodeId, NodeInfo(bestNextHop, bestDistance));
            isUpdated = true;
        }
        else
        {
            if (iter->second.nextHop != bestNextHop)
            {
                iter->second.nextHop = bestNextHop;
                isUpdated = true;
            }
            if (iter->second.distance != bestDistance)
            {
                iter->second.distance = bestDistance;
                isUpdated = true;
            }
        }
    }

    return isUpdated;
}

std::uint32_t DistanceVector::addDistance(std::uint32_t a, std::uint32_t b)
{
    if (a >= c_unreachableDistance || b >= c_unreachableDistance)
    {
        return c_unreachableDistance;
    }

    std::uint32_t result = a + b;
    if (result > c_unreachableDistance)
    {
        result = c_unreachableDistance;
    }

    return result;
}

std::optional<std::pair<std::uint32_t, csm::NodeId>> DistanceVector::distance(const NodeId& target) const
{
    std::unique_lock<std::mutex> ulock(x_dvInfos);

    auto iter = m_dvInfos.find(target);
    if (m_dvInfos.end() == iter)
    {
        return std::nullopt;
    }

    return std::make_pair(iter->second.distance, iter->second.nextHop);
}
