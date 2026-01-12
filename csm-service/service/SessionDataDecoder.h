//
// Created by chu on 7/3/25.
//

#ifndef SESSIONDATADECODER_H
#define SESSIONDATADECODER_H

#include <memory>

#include "csm-service/protocol/header/PacketHeader.h"
#include "csm-service/protocol/payload/PayloadBase.h"
#include "csm-service/protocol/payload/PayloadFactory.h"
#include "csm-service/service/P2PSession.h"
#include "csm-utilities/ThreadPool.h"

namespace csm
{

    namespace service
    {

        class SessionDataDecoder
        {
        public:
            using Ptr = std::shared_ptr<SessionDataDecoder>;

            explicit SessionDataDecoder(PayloadFactory::Ptr factory, utilities::ThreadPool::Ptr decodeWorkers);
            ~SessionDataDecoder() = default;

        public:
            // 不考虑该方法被多线程调用
            int start();

            // 不考虑该方法被多线程调用
            int stop();

            /**
             * @brief 添加recv到的数据到P2PSession中的大接收缓冲区中
             *
             * @param sessionId
             * @param data
             * @return
             */
            void addSessionData(SessionId sessionId, P2PSession::WPtr p2pSessionWeakPtr, const char* data, std::size_t dataLen);

            /**
             * @brief 设置包处理回调
             *
             * @param handler
             * @return
             */
            void setPacketHandler(
                std::function<void(SessionId sessionId, P2PSession::WPtr p2pSessionWeakPtr, PacketHeader::Ptr header, PayloadBase::Ptr payload)> handler);

        private:
            int writeDataToP2PSessionReadBuffer(SessionId sessionId, utilities::RingBuffer* readBuffer,
                                                const std::shared_ptr<std::vector<char>>& buffer);

            PacketHeader::Ptr decodePacketHeader(utilities::RingBuffer* readBuffer);
            PayloadBase::Ptr decodePacketPayload(const PacketHeader::Ptr& header, utilities::RingBuffer* readBuffer);

        private:
            PayloadFactory::Ptr m_payloadFactory;

            std::atomic_bool m_running{false};
            utilities::ThreadPool::Ptr m_decodeWorkers;

            std::function<void(SessionId sessionId, P2PSession::WPtr p2pSessionWeakPtr, PacketHeader::Ptr header, PayloadBase::Ptr payload)> m_packetHandler;
        };

    }

}


#endif //SESSIONDATADECODER_H