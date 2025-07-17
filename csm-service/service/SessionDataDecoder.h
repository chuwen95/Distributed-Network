//
// Created by chu on 7/3/25.
//

#ifndef SESSIONDATADECODER_H
#define SESSIONDATADECODER_H

#include <memory>
#include <cstring>
#include <condition_variable>

#include "csm-utilities/ThreadPool.h"
#include "csm-service/service/P2PSessionManager.h"
#include "csm-service/protocol/header/PacketHeader.h"
#include "csm-service/protocol/payload/PayloadBase.h"
#include "../protocol/payload/PayloadFactory.h"

namespace csm
{

    namespace service
    {

        class SessionDataDecoder
        {
        public:
            using Ptr = std::shared_ptr<SessionDataDecoder>;

            SessionDataDecoder(
                P2PSessionManager::Ptr p2pSessionManager, PayloadFactory::Ptr factory, utilities::ThreadPool::Ptr decodeWorkers);
            ~SessionDataDecoder() = default;

        public:
            int init();

            // 不考虑该方法被多线程调用
            int start();

            // 不考虑该方法被多线程调用
            int stop();

            /**
             * @brief 添加recv到的数据到P2PSession中的大接收缓冲区中
             *
             * @param fd
             * @param data
             * @return
             */
            int addSessionData(const int fd, const char* data, const std::size_t dataLen);

            /**
             * @brief 设置包处理回调
             *
             * @param handler
             * @return
             */
            void setPacketHandler(std::function<int(const int fd, PacketHeader::Ptr header, PayloadBase::Ptr payload)> handler);

        private:
            int writeDataToP2PSessionReadBuffer(
                const int fd, const utilities::RingBuffer::Ptr& readBuffer, const std::shared_ptr<std::vector<char>>& buffer);

            PacketHeader::Ptr decodePacketHeader(const int fd, const utilities::RingBuffer::Ptr& readBuffer);
            PayloadBase::Ptr decodePacketPayload(const int fd, const PacketHeader::Ptr& header, const utilities::RingBuffer::Ptr& readBuffer);

        private:
            P2PSessionManager::Ptr m_p2pSessionManager;
            PayloadFactory::Ptr m_payloadFactory;

            std::atomic_bool m_running{ false };
            utilities::ThreadPool::Ptr m_decodeWorkers;

            std::function<int(const int fd, PacketHeader::Ptr header, PayloadBase::Ptr payload)> m_packetHandler;
        };

    }

}



#endif //SESSIONDATADECODER_H
