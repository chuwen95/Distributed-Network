//
// Created by ChuWen on 2024/8/22.
//

#ifndef COPYSTATEMACHINE_SESSIONDATAPROCESSOR_H
#define COPYSTATEMACHINE_SESSIONDATAPROCESSOR_H

#include "csm-common/Common.h"
#include "csm-utilities/Thread.h"
#include "csm-utilities/ThreadPool.h"
#include "P2PSession.h"
#include "P2PSessionManager.h"
#include "ClientAliveChecker.h"
#include "csm-service/protocol/PayloadFactory.h"

namespace csm
{

    namespace service
    {

        class SessionDataProcessor
        {
        public:
            using Ptr = std::shared_ptr<SessionDataProcessor>;

            SessionDataProcessor(P2PSessionManager::Ptr p2pSessionManager, ClientAliveChecker::Ptr clientAliveChecker, std::size_t workerNum);
            ~SessionDataProcessor() = default;

        public:
            int init();

            int start();

            int stop();

            /**
             * @brief 添加recv到的数据到P2PSession中的大接收缓冲区中
             * @param fd
             * @param data
             * @return
             */
            int addSessionData(const int fd, const char* data, const std::size_t dataLen);

            void registerPacketHandler(const PacketType packetType, std::function<int(const int fd, PacketHeader::Ptr, PayloadBase::Ptr packet)> handler);

        private:
            int writeDataToP2PSessionReadBuffer(const int fd, utilities::RingBuffer::Ptr readBuffer, std::shared_ptr<std::vector<char>> buffer);

            int decodePackets(utilities::RingBuffer::Ptr &readBuffere, std::vector<std::pair<PacketHeader::Ptr, PayloadBase::Ptr>>& packts);

            int processPackets(const int fd, std::vector<std::pair<PacketHeader::Ptr, PayloadBase::Ptr>>& packets);

        private:
            P2PSessionManager::Ptr m_p2pSessionManager;
            ClientAliveChecker::Ptr m_clientAliveChecker;
            utilities::ThreadPool::Ptr m_worker;

            std::unordered_map<PacketType, std::function<int(const int fd, PacketHeader::Ptr, PayloadBase::Ptr packet)>> m_packetHandler;

            struct SessionData
            {
                using Ptr = std::shared_ptr<SessionData>;

                SessionData(const int f, const char* d, const std::size_t l) : fd(f)
                {
                    data.resize(l);
                    memcpy(data.data(), d, l);
                }

                int fd;
                std::vector<char> data;
            };

            std::mutex x_sessionDataInfo;
            std::condition_variable m_sessionDataInfoCv;
            std::queue<SessionData::Ptr> m_sessionDataInfo;
        };

    }

}

#endif //COPYSTATEMACHINE_SESSIONDATAPROCESSOR_H
