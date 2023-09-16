//
// Created by root on 9/5/23.
//

#ifndef TCPSERVER_TCPSESSION_H
#define TCPSERVER_TCPSESSION_H

#include "libcommon/Common.h"
#include "libcomponents/RingBuffer.h"

namespace server
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

        void setClientInfo(const std::string_view id);

        void getClientInfo(std::string& id);

    public:
        int m_clientfd{-1};

        struct ClientInfo
        {
            ClientInfo() = default;
            ClientInfo(const std::string_view idParam) : id(idParam) {}

            std::string id;
        };
        ClientInfo m_clientInfo;

        components::RingBuffer::Ptr m_readBuffer;
        components::RingBuffer::Ptr m_writeBuffer;
    };

} // server

#endif //TCPSERVER_TCPSESSION_H
