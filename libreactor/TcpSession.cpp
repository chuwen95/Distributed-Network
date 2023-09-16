//
// Created by root on 9/5/23.
//

#include "TcpSession.h"

#include "libcomponents/Socket.h"

namespace server
{

    TcpSession::TcpSession()
    {
        m_readBuffer = std::make_shared<components::RingBuffer>();
        m_writeBuffer = std::make_shared<components::RingBuffer>();
    }

    int TcpSession::init(const int fd)
    {
        m_clientfd = fd;
        components::Socket::setNonBlock(m_clientfd);

        m_readBuffer->init(4 * 1024 * 1024);
        m_writeBuffer->init(4 * 1024 * 1024);

        return 0;
    }

    int TcpSession::uninit()
    {
        return 0;
    }

    int TcpSession::start()
    {
        return 0;
    }

    int TcpSession::stop()
    {
        return 0;
    }

    int TcpSession::fd()
    {
        return m_clientfd;
    }

    components::RingBuffer::Ptr TcpSession::readBuffer()
    {
        return m_readBuffer;
    }

    components::RingBuffer::Ptr TcpSession::writeBuffer()
    {
        return m_writeBuffer;
    }

    void TcpSession::setClientInfo(const std::string_view id)
    {
        m_clientInfo.id = id;
    }

    void TcpSession::getClientInfo(std::string &id)
    {
        id = m_clientInfo.id;
    }

} // server