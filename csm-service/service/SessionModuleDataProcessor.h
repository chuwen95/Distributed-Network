//
// Created by ChuWen on 2024/8/22.
//

#ifndef COPYSTATEMACHINE_SESSIONDATAPROCESSOR_H
#define COPYSTATEMACHINE_SESSIONDATAPROCESSOR_H

#include "csm-common/Common.h"
#include "csm-service/protocol/header/PacketHeader.h"
#include "csm-service/protocol/payload/PayloadFactory.h"
#include "P2PSession.h"
#include "csm-utilities/ThreadPool.h"

namespace csm
{

    namespace service
    {

        class SessionModuleDataProcessor
        {
        public:
            using Ptr = std::shared_ptr<SessionModuleDataProcessor>;

            explicit SessionModuleDataProcessor(utilities::ThreadPool::Ptr normalPacketProcessor);

        public:
            int start();

            int stop();

            void addPacket(SessionId sessionId, P2PSession::WPtr p2pSessionWeakPtr, PacketHeader::Ptr header,
                           PayloadBase::Ptr payload);

            /**
             * @brief 注册包处理回调
             * @param packetType
             * @param handler
             */
            void registerPacketHandler(PacketType packetType,
                                       std::function<void(SessionId, P2PSession::WPtr p2pSessionWeakPtr, PacketHeader::Ptr,
                                                          PayloadBase::Ptr)> handler);

        private:
            utilities::ThreadPool::Ptr m_normalPacketProcessor;

            std::unordered_map<PacketType, std::function<void(SessionId, P2PSession::WPtr, PacketHeader::Ptr,
                                                              PayloadBase::Ptr)>> m_packetHandler;
        };

    } // namespace service

} // namespace csm

#endif // COPYSTATEMACHINE_SESSIONDATAPROCESSOR_H