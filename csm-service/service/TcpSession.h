//
// Created by ChuWen on 9/5/23.
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

            /**
            * @brief 获取客户端上线的时间戳，供业务线程池用，业务线程池放入任务的时候会记录放入任务那一刻的时间戳
            * 如果客户端不存在此函数返回-1，表示客户端已经离线，直接丢弃任务
            * 如果任务时间戳小于客户端上线时间戳，可能是客户端离线后又上线但是fd分配的是一样的
            * @return
            */
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
