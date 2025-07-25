//
// Created by ChuWen on 9/5/23.
//

#ifndef P2PSESSION_H
#define P2PSESSION_H

#include "csm-common/Common.h"
#include "csm-framework/cluster/Common.h"
#include "csm-service/host/HostEndPointInfo.h"
#include "csm-utilities/RingBuffer.h"

namespace csm
{

    namespace service
    {

        constexpr std::size_t c_p2pSessionReadBufferSize{16 * 1024};
        constexpr std::size_t c_p2pSessionWriteBufferSize{16 * 1024};

        class P2PSession
        {
        public:
            using Ptr = std::shared_ptr<P2PSession>;

            P2PSession(int fd, utilities::RingBuffer::Ptr readBuffer, utilities::RingBuffer::Ptr writeBuffer);
            ~P2PSession() = default;

        public:
            int init();

            int fd() const;

            std::mutex& readBufferMutex();
            utilities::RingBuffer::Ptr& readBuffer();

            std::mutex& writeBufferMutex();
            utilities::RingBuffer::Ptr& writeBuffer();

            void setNodeId(std::string id);
            std::string nodeId() const;

            void setNodeOnlineTimestamp(std::uint32_t timestamp);
            /**
             * @brief 获取客户端上线的时间戳，供业务线程池用，业务线程池放入任务的时候会记录放入任务那一刻的时间戳
             * 如果客户端不存在此函数返回-1，表示客户端已经离线，直接丢弃任务
             * 如果任务时间戳小于客户端上线时间戳，可能是客户端离线后又上线但是fd分配的是一样的
             * @return
             */
            std::uint32_t clientOnlineTimestamp() const;

            template <typename... Args> void setPeerHostEndPointInfo(Args... args)
            {
                m_peerHostEndPointInfo = HostEndPointInfo(std::forward<Args>(args)...);
            }

            void setPeerHostEndPointInfo(const HostEndPointInfo& hostEndPointInfo);

            const HostEndPointInfo& peerHostEndPointInfo() const;

            void setHandshakeUuid(std::string uuid);
            std::string handshakeUuid() const;

            bool isWaitingDisconnect() const;
            void setWaitingDisconnect(bool value);

        public:
            int m_fd{-1};

            struct NodeInfo
            {
                NodeInfo() = default;
                NodeInfo(const std::string_view idParam, const std::uint32_t ts) : id(idParam), onlineTimestamp(ts) {}

                NodeId id;
                std::uint32_t onlineTimestamp{0};
            };
            NodeInfo m_nodeInfo;

            std::mutex x_readBuffer;
            utilities::RingBuffer::Ptr m_readBuffer;
            std::mutex x_writeBuffer;
            utilities::RingBuffer::Ptr m_writeBuffer;

            HostEndPointInfo m_peerHostEndPointInfo;
            std::string m_handshakeUuid;

            std::atomic_bool m_isWaitingDisconnect{ false };
        };

    } // server

}

#endif //P2PSESSION_H
