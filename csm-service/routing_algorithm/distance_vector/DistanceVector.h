//
// Created by ivy on 1/8/26.
//

#ifndef COPYSTATEMACHINE_DISTANCEVECTOR_H
#define COPYSTATEMACHINE_DISTANCEVECTOR_H

#include <memory>
#include <mutex>
#include <optional>
#include <unordered_map>

#include "csm-framework/cluster/Common.h"

namespace csm
{

    namespace service
    {

        const std::uint32_t c_unreachableDistance{750};

        class DistanceVector
        {
        public:
            DistanceVector(const NodeIds& nodeIds);

        public:
            // 获取所有邻居节点
            NodeIds neighbours() const;

            // 更新与邻居节点的距离
            bool updateNeighbourDistance(const NodeId& peerNodeId, std::uint32_t distance);
            // 更新节点距离向量信息
            bool updateDvInfos(const NodeId& peerNodeId, const std::vector<std::pair<NodeId, std::uint32_t>>& peerDvInfos);

            // 获取针对某个节点的距离向量
            std::vector<std::pair<csm::NodeId, std::uint32_t>> dvInfo(const NodeId& peerNodeId);

            // 当前的距离向量表（for unittest）(返回值：目标节点，距离，下一跳)
            std::vector<std::tuple<csm::NodeId, std::uint32_t, csm::NodeId>> dvInfos();

            std::optional<std::pair<std::uint32_t, NodeId>> distance(const NodeId& target);

        private:
            struct NodeInfo
            {
                NodeInfo() = default;
                explicit NodeInfo(NodeId n) : nextHop(std::move(n)) {}
                explicit NodeInfo(NodeId n, std::uint32_t distance) : nextHop(std::move(n)), distance(distance) {}

                NodeId nextHop{c_invalidNodeId}; // 到达目标节点的下一跳节点
                std::uint32_t distance{c_unreachableDistance}; // 到达目标节点的距离
            };
            std::shared_ptr<NodeInfo> queryNodeInfoWithoutLock(const NodeId& nodeId);

        private:
            std::mutex x_dvInfos;
            std::unordered_map<NodeId, std::shared_ptr<NodeInfo>> m_dvInfos;

            // NeighbourNodeId => { TargetNodeId, distance }
            std::unordered_map<NodeId, std::unordered_map<NodeId, std::uint32_t>> m_neighboursDVInfo;
        };

    }

}


#endif //COPYSTATEMACHINE_DISTANCEVECTOR_H