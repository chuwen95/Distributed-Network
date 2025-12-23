//
// Created by chu on 7/7/25.
//

#ifndef SESSIONSERVICEDATAPROCESSOR_H
#define SESSIONSERVICEDATAPROCESSOR_H

#include "csm-common/Common.h"

#include "concurrentqueue/moodycamel/blockingconcurrentqueue.h"

#include "csm-service/protocol/header/PacketHeader.h"
#include "csm-service/protocol/payload/PayloadBase.h"
#include "P2PSession.h"
#include "csm-utilities/Thread.h"

namespace csm
{
    namespace service
    {
        class SessionServiceDataProcessor
        {
        public:
            int init();

            // 不考虑该方法被多线程调用
            int start();

            // 不考虑该方法被多线程调用
            int stop();

            int addPacket(SessionId sessionId, P2PSession::WPtr p2pSession, PacketHeader::Ptr header,
                          PayloadBase::Ptr payload);

            void registerPacketHandler(PacketType packetType,
                                       std::function<void(SessionId sessionId, P2PSession::WPtr p2pSession,
                                                          PacketHeader::Ptr header, PayloadBase::Ptr payload)> handler);

        private:
            struct SessionServiceData
            {
                using Ptr = std::shared_ptr<SessionServiceData>;

                explicit SessionServiceData(SessionId si, P2PSession::WPtr ps, PacketHeader::Ptr ph,
                                            PayloadBase::Ptr pb)
                    : sessionId(si), p2pSessionWeakPtr(std::move(ps)), header(std::move(ph)), payload(std::move(pb))
                {
                }

                SessionId sessionId;
                P2PSession::WPtr p2pSessionWeakPtr;
                PacketHeader::Ptr header;
                PayloadBase::Ptr payload;
            };

            moodycamel::BlockingConcurrentQueue<SessionServiceData::Ptr> m_sessionServiceDatas;

            std::atomic_bool m_running{false};

            std::unordered_map<PacketType, std::function<void(SessionId, P2PSession::WPtr, PacketHeader::Ptr,
                                                              PayloadBase::Ptr)>> m_packetHandlers;
            std::unique_ptr<utilities::Thread> m_thread;
        };
    }
}

#endif //SESSIONSERVICEDATAPROCESSOR_H
