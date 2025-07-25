//
// Created by chu on 7/9/25.
//

#ifndef DISTANCEVECTOR_H
#define DISTANCEVECTOR_H

#include <memory>
#include <mutex>
#include <unordered_map>

#include "csm-framework/cluster/Common.h"
#include "csm-service/protocol/header/PacketHeader.h"
#include "csm-service/protocol/payload/PayloadBase.h"
#include "csm-utilities/Thread.h"

namespace csm
{

    namespace service
    {

        const std::uint32_t c_unreachableDistance{750};

        class DistanceVector
        {
        public:
            using Ptr = std::shared_ptr<DistanceVector>;

            explicit DistanceVector(utilities::Thread::Ptr thread);
            ~DistanceVector() = default;

        public:
            int init();

            int start();

            void stop();

            /**
             * @brief 设置网络数据包发送器
             *
             * @param sender
             */
            void setPacketSender(std::function<int(const NodeId& nodeId, const std::vector<char>& data)> sender);

            /**
             *@brief 添加邻居节点
             *
             * @param nodeId
             */
            void addNeighbourNode(const NodeId& nodeId);

            /**
             * @brief 设置邻居节点不可达
             *
             * @param nodeId
             * @return
             */
            int setNightbourUnreachable(const NodeId& nodeId);

            /**
             * @brief 处理网络数据包
             *
             * @param header
             * @param payload
             * @return
             */
            int handlePacket(const NodeId& fromNodeId, PacketHeader::Ptr header, PayloadBase::Ptr payload);

        private:
            void sendDistanceDetect();
            void sendDistanceVector();

            int handleDistanceDetect(const NodeId& fromNodeId, const PayloadBase::Ptr& payload);
            int handleDistanceDetectReply(const NodeId& fromNodeId, const PayloadBase::Ptr& payload);
            int handleDistanceVector(const NodeId& fromNodeId, const PayloadBase::Ptr& payload);

            bool queryNodeDistanceWithoutLock(const NodeId& nodeId, std::uint32_t& distance);

        private:
            struct NodeInfo
            {
                using Ptr = std::shared_ptr<NodeInfo>;

                NodeInfo() = default;
                explicit NodeInfo(NodeId n) : nextHop(std::move(n)) {}

                NodeId nextHop; // 到达目标节点的下一跳节点
                std::uint32_t distance{c_unreachableDistance}; // 到达目标节点的距离
            };

            std::mutex x_dvInfo;
            std::set<NodeId> m_neighbours;
            std::unordered_map<NodeId, NodeInfo::Ptr> m_dvInfos;

            std::function<int(const NodeId& nodeId, const std::vector<char>& data)> m_packetSender;
            utilities::Thread::Ptr m_thread{nullptr};

            std::uint64_t m_distanceDetectSeq{0};
            std::atomic_bool m_needToSyncDistaceVector{false};
        };

    }

}

#endif //DISTANCEVECTOR_H