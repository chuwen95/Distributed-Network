//
// Created by root on 9/5/23.
//

#include "TcpSession.h"

#include "libcomponents/Socket.h"

namespace service
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

    void TcpSession::setClientId(const std::string_view id)
    {
        m_clientInfo.id = id;
    }

    std::string TcpSession::getClientId()
    {
        return m_clientInfo.id;
    }

    void TcpSession::setClientOnlineTimestamp(const std::uint32_t timestamp)
    {
        m_clientInfo.onlineTimestamp = timestamp;
    }

    std::uint32_t TcpSession::getClientOnlineTimestamp()
    {
        return m_clientInfo.onlineTimestamp;
    }

    void TcpSession::setPeerHostEndPointInfo(const service::HostEndPointInfo &hostEndPointInfo)
    {
        m_peerHostEndPointInfo = hostEndPointInfo;
    }

    const HostEndPointInfo& TcpSession::peerHostEndPointInfo() const
    {
        return m_peerHostEndPointInfo;
    }

    void TcpSession::setHandshakeUuid(const std::string &uuid)
    {
        m_handshakeUuid = uuid;
    }

    std::string TcpSession::handshakeUuid()
    {
        return m_handshakeUuid;
    }

} // server