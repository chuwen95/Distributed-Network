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

    public:
        int m_clientfd{-1};

        components::RingBuffer::Ptr m_readBuffer;
        components::RingBuffer::Ptr m_writeBuffer;
    };

} // server

#endif //TCPSERVER_TCPSESSION_H
