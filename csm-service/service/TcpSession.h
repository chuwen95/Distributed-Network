//
// Created by root on 9/5/23.
//

#ifndef TCPSERVER_TCPSESSION_H
#define TCPSERVER_TCPSESSION_H

#include "csm-common/Common.h"
#include "csm-utilities/RingBuffer.h"
#include "csm-service/host/HostEndPointInfo.h"

namespace csm
{

    namespace service
    {

        class TcpSession
        {
        public:
            using Ptr = std::shared_ptr<TcpSession>;

            TcpSession(const int fd, utilities::RingBuffer::Ptr readBuffer, utilities::RingBuffer::Ptr writeBuffer);
            ~TcpSession() = default;

        public:
            int init();

            int uninit();

            int start();

            int stop();

            int fd();

            utilities::RingBuffer::Ptr &readBuffer();

            utilities::RingBuffer::Ptr &writeBuffer();

            void setClientId(const std::string_view id);

            std::string getClientId();

            void setClientOnlineTimestamp(const std::uint32_t timestamp);

            std::uint32_t getClientOnlineTimestamp();

            template<typename... Args>
            void setPeerHostEndPointInfo(Args... args)
            {
                m_peerHostEndPointInfo = std::move(HostEndPointInfo(std::forward<Args>(args)...));
            }

            void setPeerHostEndPointInfo(const HostEndPointInfo &hostEndPointInfo);

            const HostEndPointInfo &peerHostEndPointInfo() const;

            void setHandshakeUuid(const std::string &uuid);

            std::string handshakeUuid();

        public:
            int m_clientfd{-1};

            struct ClientInfo
            {
                ClientInfo() = default;

                ClientInfo(const std::string_view idParam, const std::uint32_t ts) : id(idParam), onlineTimestamp(ts)
                {}

                std::string id;
                std::uint32_t onlineTimestamp;
            };

            ClientInfo m_clientInfo;

            utilities::RingBuffer::Ptr m_readBuffer;
            utilities::RingBuffer::Ptr m_writeBuffer;

            HostEndPointInfo m_peerHostEndPointInfo;
            std::string m_handshakeUuid;
        };

    } // server

}

#endif //TCPSERVER_TCPSESSION_H
