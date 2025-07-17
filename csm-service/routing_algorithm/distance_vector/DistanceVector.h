//
// Created by chu on 7/9/25.
//

#ifndef DISTANCEVECTOR_H
#define DISTANCEVECTOR_H

#include <mutex>
#include <memory>
#include <unordered_map>

#include "csm-framework/cluster/Common.h"
#include "csm-service/protocol/header/PacketHeader.h"
#include "csm-service/protocol/payload/PayloadBase.h"
#include "csm-utilities/Thread.h"

namespace csm
{

    namespace service
    {

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
            void addNeighbourNode(const NodeId& nodeId, int fd);

            /**
             * @brief 处理网络数据包
             *
             * @param header
             * @param payload
             * @return
             */
            int handlePacket(const NodeId& fromNodeId, PacketHeader::Ptr header, PayloadBase::Ptr payload);

        private:
            int handleDistanceDetect(const NodeId& fromNodeId, const PacketHeader::Ptr& header, const PayloadBase::Ptr& payload);
            int handleDistanceDetectReply(const NodeId& fromNodeId, const PacketHeader::Ptr& header, const PayloadBase::Ptr& payload);

        private:
            struct NodeInfo
            {
                using Ptr = std::shared_ptr<NodeInfo>;

                NodeInfo() = default;
                NodeInfo(NodeId ni, NodeId nh) :
                    nodeId(std::move(ni)), nextHop(std::move(nh)) {}

                [[nodiscard]] bool isNeighbourhood() const { return nodeId == nextHop; }

                NodeId nodeId;     // 目标节点
                NodeId nextHop;    // 到达目标节点的下一跳节点
                int distance{ -1 };     // 到达目标节点的距离
            };
            std::mutex x_dvInfo;
            std::unordered_map<NodeId, NodeInfo::Ptr> m_dvInfo;

            std::function<int(const NodeId& nodeId, const std::vector<char>& data)> m_packetSender;
            utilities::Thread::Ptr m_thread{ nullptr };

            std::uint64_t m_distanceDetectSeq{0};
        };

    }

}

#endif //DISTANCEVECTOR_H