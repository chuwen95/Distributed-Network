//
// Created by ChuWen on 2024/8/22.
//

#ifndef COPYSTATEMACHINE_SESSIONDATAPROCESSOR_H
#define COPYSTATEMACHINE_SESSIONDATAPROCESSOR_H

#include "P2PSessionManager.h"
#include "csm-common/Common.h"
#include "csm-service/protocol/header/PacketHeader.h"
#include "csm-service/protocol/payload/PayloadFactory.h"
#include "csm-utilities/ThreadPool.h"

namespace csm
{

    namespace service
    {

        class SessionModuleDataProcessor
        {
        public:
            using Ptr = std::shared_ptr<SessionModuleDataProcessor>;

            SessionModuleDataProcessor(P2PSessionManager::Ptr p2pSessionManager,
                                       utilities::ThreadPool::Ptr normalPacketProcessor);
            ~SessionModuleDataProcessor() = default;

        public:
            int init();

            int start();

            int stop();

            void addPacket(const int fd, PacketHeader::Ptr header, PayloadBase::Ptr payload);

            /**
             * @brief 注册包处理回调
             * @param packetType
             * @param handler
             */
            void registerPacketHandler(PacketType packetType,
                                       std::function<int(const int fd, PacketHeader::Ptr, PayloadBase::Ptr packet)> handler);

        private:
            int processPackets(int fd, PacketHeader::Ptr header, PayloadBase::Ptr payload);

        private:
            P2PSessionManager::Ptr m_p2pSessionManager;
            utilities::ThreadPool::Ptr m_normalPacketProcessor;

            std::unordered_map<PacketType, std::function<int(const int fd, PacketHeader::Ptr, PayloadBase::Ptr packet)>>
                m_packetHandler;
        };

    } // namespace service

} // namespace csm

#endif // COPYSTATEMACHINE_SESSIONDATAPROCESSOR_H
