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
            explicit DistanceVector(const NodeIds& nodes);
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
            std::vector<NodeId> m_neighbours;

            struct NodeInfo
            {
                NodeInfo() = default;
                explicit NodeInfo(NodeId n) : nextHop(std::move(n)) {}

                NodeId nextHop; // 到达目标节点的下一跳节点
                std::uint32_t distance{c_unreachableDistance}; // 到达目标节点的距离
            };

            std::mutex x_dvInfos;
            std::unordered_map<NodeId, NodeInfo> m_dvInfos;

            std::unordered_map<NodeId, std::unordered_map<NodeId, std::uint32_t>> m_neighboursDVInfo;

            std::function<int(const NodeId& nodeId, const std::vector<char>& data)> m_packetSender;
            std::unique_ptr<utilities::Thread> m_thread{nullptr};

            std::uint64_t m_distanceDetectSeq{0};
            std::atomic_bool m_needToSyncDistaceVector{false};
        };

    }

}

#endif //DISTANCEVECTOR_H