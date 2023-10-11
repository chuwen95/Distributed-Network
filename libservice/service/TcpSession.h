//
// Created by root on 9/5/23.
//

#ifndef TCPSERVER_TCPSESSION_H
#define TCPSERVER_TCPSESSION_H

#include "libcommon/Common.h"
#include "libcomponents/RingBuffer.h"
#include "../host/HostEndPointInfo.h"

namespace service
{

    class TcpSession
    {
    public:
        using Ptr = std::shared_ptr<TcpSession>;

        TcpSession();
        ~TcpSession() = default;

    public:
        int init(const int fd);

        int uninit();

        int start();

        int stop();

        int fd();

        components::RingBuffer::Ptr readBuffer();
        components::RingBuffer::Ptr writeBuffer();

        void setClientId(const std::string_view id);
        std::string getClientId();

        void setClientOnlineTimestamp(const std::uint32_t timestamp);
        std::uint32_t getClientOnlineTimestamp();

        template<typename... Args>
        void setPeerHostEndPointInfo(Args... args)
        {
            m_peerHostEndPointInfo = std::move(HostEndPointInfo(std::forward<Args>(args)...));
        }
        void setPeerHostEndPointInfo(const HostEndPointInfo& hostEndPointInfo);
        const HostEndPointInfo& peerHostEndPointInfo() const;

        void setHandshakeUuid(const std::string& uuid);
        std::string handshakeUuid();

    public:
        int m_clientfd{-1};

        struct ClientInfo
        {
            ClientInfo() = default;
            ClientInfo(const std::string_view idParam, const std::uint32_t ts) : id(idParam), onlineTimestamp(ts) {}

            std::string id;
            std::uint32_t onlineTimestamp;
        };
        ClientInfo m_clientInfo;

        components::RingBuffer::Ptr m_readBuffer;
        components::RingBuffer::Ptr m_writeBuffer;

        HostEndPointInfo m_peerHostEndPointInfo;
        std::string m_handshakeUuid;
    };

} // server

#endif //TCPSERVER_TCPSESSION_H
