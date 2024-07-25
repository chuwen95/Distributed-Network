//
// Created by root on 9/5/23.
//

#include "TcpSession.h"

#include "csm-utilities/Socket.h"

using namespace csm::service;

TcpSession::TcpSession(const int fd, utilities::RingBuffer::Ptr readBuffer, utilities::RingBuffer::Ptr writeBuffer) :
        m_clientfd(fd), m_readBuffer(std::move(readBuffer)), m_writeBuffer(std::move(writeBuffer))
{
}

int TcpSession::init()
{
    utilities::Socket::setNonBlock(m_clientfd);

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

csm::utilities::RingBuffer::Ptr &TcpSession::readBuffer()
{
    return m_readBuffer;
}

csm::utilities::RingBuffer::Ptr &TcpSession::writeBuffer()
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

const HostEndPointInfo &TcpSession::peerHostEndPointInfo() const
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