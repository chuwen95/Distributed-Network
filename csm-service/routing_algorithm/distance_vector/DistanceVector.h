//
// Created by ivy on 1/8/26.
//

#ifndef COPYSTATEMACHINE_DISTANCEVECTOR_H
#define COPYSTATEMACHINE_DISTANCEVECTOR_H

#include <mutex>
#include <optional>
#include <unordered_map>

#include "csm-framework/cluster/Common.h"

#include "DistanceVectorCommon.h"

namespace csm
{

    namespace service
    {

        class DistanceVector
        {
        public:
            DistanceVector() = default;
            DistanceVector(NodeId selfNodeId, const NodeIds& nodeIds);

        public:
            // 获取所有邻居节点
            NodeIds neighbours() const;

            // 更新与邻居节点的距离
            bool updateNeighbourDistance(const NodeId& peerNodeId, Distance distance);
            // 更新节点距离向量信息
            bool updateDvInfos(const NodeId& peerNodeId, const std::vector<std::pair<NodeId, Distance>>& peerDvInfos);

            // 获取针对某个节点的距离向量
            std::vector<std::pair<csm::NodeId, Distance>> dvInfo(const NodeId& peerNodeId) const;

            // 当前的距离向量表（for unittest）(返回值：目标节点，距离，下一跳)
            std::vector<std::tuple<csm::NodeId, Distance, csm::NodeId>> dvInfos() const;

            std::optional<std::pair<Distance, NodeId>> distance(const NodeId& target) const;

        private:
            bool recomputeRoutesWithoutLock();
            Distance addDistance(Distance a, Distance b);

        private:
            NodeId m_selfNodeId;

            struct NodeInfo
            {
                NodeInfo() = default;
                explicit NodeInfo(NodeId n) : nextHop(std::move(n)) {}
                explicit NodeInfo(NodeId n, Distance distance) : nextHop(std::move(n)), distance(distance) {}

                NodeId nextHop{c_invalidNodeId}; // 到达目标节点的下一跳节点
                Distance distance{c_unreachableDistance}; // 到达目标节点的距离
            };

            std::unordered_map<NodeId, NodeInfo> m_dvInfos;

            // NeighbourNodeId => { TargetNodeId, distance }
            std::unordered_map<NodeId, std::unordered_map<NodeId, Distance>> m_neighboursDVInfo;
        };

    }

}


#endif //COPYSTATEMACHINE_DISTANCEVECTOR_H