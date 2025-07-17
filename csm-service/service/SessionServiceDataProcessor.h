//
// Created by chu on 7/7/25.
//

#ifndef SESSIONSERVICEDATAPROCESSOR_H
#define SESSIONSERVICEDATAPROCESSOR_H

#include "csm-common/Common.h"

#include <concurrentqueue/blockingconcurrentqueue.h>

#include "csm-service/protocol/header/PacketHeader.h"
#include "csm-service/protocol/payload/PayloadBase.h"
#include "csm-utilities/Thread.h"

namespace csm
{
    namespace service
    {

        class SessionServiceDataProcessor
        {
        public:
            using Ptr = std::shared_ptr<SessionServiceDataProcessor>;

            SessionServiceDataProcessor(utilities::Thread::Ptr thread);
            ~SessionServiceDataProcessor() = default;

        public:
            int init();

            // 不考虑该方法被多线程调用
            int start();

            // 不考虑该方法被多线程调用
            int stop();

            int addPacket(const int fd, PacketHeader::Ptr header, PayloadBase::Ptr payload);

            void registerPacketHandler(const PacketType packetType, std::function<int(int fd, PacketHeader::Ptr header, PayloadBase::Ptr payload)> handler);

        private:
            struct SessionServiceData
            {
                SessionServiceData() = default;
                SessionServiceData(const int f, PacketHeader::Ptr h, PayloadBase::Ptr p) :
                    fd(f), header(std::move(h)), payload(std::move(p)) {}

                int fd;
                PacketHeader::Ptr header;
                PayloadBase::Ptr payload;
            };
            moodycamel::BlockingConcurrentQueue<SessionServiceData> m_sessionServiceDatas;

            std::atomic_bool m_running{false};

            std::unordered_map<PacketType, std::function<int(int fd, PacketHeader::Ptr header, PayloadBase::Ptr payload)>> m_packetHandlers;
            utilities::Thread::Ptr m_thread;
        };

    }
}

#endif //SESSIONSERVICEDATAPROCESSOR_H