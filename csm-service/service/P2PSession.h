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

        using SessionId = std::uint64_t;

        class P2PSession
        {
        public:
            using Ptr = std::shared_ptr<P2PSession>;
            using WPtr = std::weak_ptr<P2PSession>;

            explicit P2PSession(int fd, SessionId sessionId, utilities::RingBuffer::Ptr readBuffer,
                                utilities::RingBuffer::Ptr writeBuffer);

        public:
            int init();

            int fd() const;
            SessionId sessionId() const;

            std::mutex& readBufferMutex();
            utilities::RingBuffer::Ptr& readBuffer();

            std::mutex& writeBufferMutex();
            utilities::RingBuffer::Ptr& writeBuffer();

            void setNodeId(NodeId id);
            NodeId nodeId() const;

            template <typename... Args> void setPeerHostEndPointInfo(Args... args)
            {
                m_peerHostEndPointInfo = HostEndPointInfo(std::forward<Args>(args)...);
            }

            void setPeerHostEndPointInfo(const HostEndPointInfo& hostEndPointInfo);

            const HostEndPointInfo& peerHostEndPointInfo() const;

            bool isWaitingDisconnect() const;
            void setWaitingDisconnect(bool value);

            void setSlaveReactorIndex(std::size_t index);
            std::size_t slaveReactorIndex() const;

            bool isHeartbeatTimeout();
            void refreshHeartbeat();

        public:
            const int m_fd{-1};
            const SessionId m_sessionId{0};

            NodeId m_nodeId;

            std::mutex x_readBuffer;
            utilities::RingBuffer::Ptr m_readBuffer;
            std::mutex x_writeBuffer;
            utilities::RingBuffer::Ptr m_writeBuffer;

            HostEndPointInfo m_peerHostEndPointInfo;

            std::atomic_bool m_isWaitingDisconnect{false};

            std::size_t m_slaveReactorIndex;

            int m_lastRecvCount{0};
            int m_recvCount{1};
        };

    } // server

}

#endif //P2PSESSION_H