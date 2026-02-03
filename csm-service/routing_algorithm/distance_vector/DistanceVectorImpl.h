//
// Created by chu on 7/9/25.
//

#ifndef DISTANCEVECTORIMPL_H
#define DISTANCEVECTORIMPL_H

#include <memory>

#include "DistanceVector.h"
#include "csm-service/protocol/header/PacketHeader.h"
#include "csm-service/protocol/payload/PayloadBase.h"
#include "csm-utilities/Thread.h"

namespace csm
{

    namespace service
    {

        class DistanceVectorImpl
        {
        public:
            explicit DistanceVectorImpl(const NodeIds& nodes);
            ~DistanceVectorImpl() = default;

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

        private:
            DistanceVector m_distanceVector;

            std::function<int(const NodeId& nodeId, const std::vector<char>& data)> m_packetSender;
            std::unique_ptr<utilities::Thread> m_thread{nullptr};

            std::uint64_t m_distanceDetectSeq{0};
            std::atomic_bool m_needToSyncDistaceVector{false};
        };

    }

}

#endif //DISTANCEVECTOR_H