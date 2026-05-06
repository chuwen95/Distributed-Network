//
// Created by chu on 7/9/25.
//

#ifndef DISTANCEVECTORIMPL_H
#define DISTANCEVECTORIMPL_H

#include <memory>
#include <variant>

#include <concurrentqueue/moodycamel/blockingconcurrentqueue.h>

#include "DistanceVectorServiceInterface.h"
#include "DistanceVector.h"

#include "csm-service/protocol/header/PacketHeader.h"
#include "csm-service/protocol/payload/PayloadDistanceDetect.h"
#include "csm-service/protocol/payload/PayloadDistanceDetectReply.h"
#include "csm-service/protocol/payload/PayloadDistanceVector.h"

#include "csm-utilities/Thread.h"
#include "csm-utilities/ElapsedTime.h"

namespace csm
{
    namespace service
    {
        class DistanceVectorService : public DistanceVectorServiceInterface
        {
        public:
            explicit DistanceVectorService(NodeId selfNodeId, NodeIds nodeIds);
            ~DistanceVectorService() override = default;

        public:
            void start() override;

            void stop() override;

            /**
             * @brief 设置网络数据包发送器
             *
             * @param sender
             */
            void setPacketSender(
                std::function<int(const NodeId& nodeId, std::shared_ptr<std::vector<char>> data)> sender) override;

            /**
             * @brief 处理网络数据包
             *
             * @param header
             * @param payload
             * @return
             */
            int handlePacket(NodeId fromNodeId, PacketHeader::Ptr header, PayloadBase::Ptr payload) override;

            std::optional<std::pair<Distance, NodeId>> queryRoute(const NodeId& targetNodeId) override;

        private:
            void sendDistanceDetect();
            void sendDistanceVector();

        private:
            struct DistanceDetectEvent
            {
                NodeId fromNodeId;
                PayloadDistanceDetect::Ptr payloadDistanceDetect;
            };

            struct DistanceDetectReplyEvent
            {
                NodeId fromNodeId;
                PayloadDistanceDetectReply::Ptr payloadDistanceDetectReply;
            };

            struct DistanceVectorEvent
            {
                NodeId fromNodeId;
                PayloadDistanceVector::Ptr payloadDistanceVector;
            };

            using Event = std::variant<DistanceDetectReplyEvent, DistanceVectorEvent>;

        private:
            template <class... Ts>
            struct Overloaded : Ts...
            {
                using Ts::operator()...;
            };

            template <class... Ts>
            Overloaded(Ts...) -> Overloaded<Ts...>;

            int handleDistanceDetect(DistanceDetectEvent event);
            int handleDistanceDetectReply(DistanceDetectReplyEvent event);
            int handleDistanceVector(DistanceVectorEvent event);

            void initDistanceDetectSeqInfo();
            void initEventHandler();

        private:
            std::atomic<std::shared_ptr<const DistanceVector>> m_distanceVector;

            utilities::ElapsedTime m_distanceDetectElapsedTime;

            bool m_isEventTriggerDistanceVectorBoardcast{false};
            utilities::ElapsedTime m_distanceVectorElapsedTime;

            struct DistanceDetectSendReplyInfo
            {
                std::uint64_t sendSeq{0};
                std::uint64_t receivedSeq{0};
            };

            std::unordered_map<NodeId, DistanceDetectSendReplyInfo> m_neighbourDistanceDetectSeqInfo;
            std::function<int(const NodeId& nodeId, std::shared_ptr<std::vector<char>> data)> m_packetSender;

            moodycamel::BlockingConcurrentQueue<Event> m_eventQueue;
            std::unique_ptr<utilities::Thread> m_thread{nullptr};
        };
    }
}

#endif //DISTANCEVECTOR_H
