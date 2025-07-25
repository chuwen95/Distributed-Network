//
// Created by ChuWen on 9/5/23.
//

#include "P2PSession.h"

#include "csm-utilities/Socket.h"

using namespace csm::service;

P2PSession::P2PSession(const int fd, utilities::RingBuffer::Ptr readBuffer, utilities::RingBuffer::Ptr writeBuffer)
    : m_fd(fd), m_readBuffer(std::move(readBuffer)), m_writeBuffer(std::move(writeBuffer))
{
}

int P2PSession::init()
{
    utilities::Socket::setNonBlock(m_fd);

    return 0;
}

int P2PSession::fd() const
{
    return m_fd;
}

std::mutex& P2PSession::readBufferMutex()
{
    return x_readBuffer;
}

csm::utilities::RingBuffer::Ptr& P2PSession::readBuffer()
{
    return m_readBuffer;
}

std::mutex& P2PSession::writeBufferMutex()
{
    return x_writeBuffer;
}

csm::utilities::RingBuffer::Ptr& P2PSession::writeBuffer()
{
    return m_writeBuffer;
}

void P2PSession::setNodeId(std::string id)
{
    m_nodeInfo.id = std::move(id);
}

std::string P2PSession::nodeId() const
{
    return m_nodeInfo.id;
}

void P2PSession::setNodeOnlineTimestamp(const std::uint32_t timestamp)
{
    m_nodeInfo.onlineTimestamp = timestamp;
}

std::uint32_t P2PSession::clientOnlineTimestamp() const
{
    return m_nodeInfo.onlineTimestamp;
}

void P2PSession::setPeerHostEndPointInfo(const service::HostEndPointInfo& hostEndPointInfo)
{
    m_peerHostEndPointInfo = hostEndPointInfo;
}

const HostEndPointInfo& P2PSession::peerHostEndPointInfo() const
{
    return m_peerHostEndPointInfo;
}

void P2PSession::setHandshakeUuid(std::string uuid)
{
    m_handshakeUuid = std::move(uuid);
}

std::string P2PSession::handshakeUuid() const
{
    return m_handshakeUuid;
}

bool P2PSession::isWaitingDisconnect() const
{
    return m_isWaitingDisconnect;
}

void P2PSession::setWaitingDisconnect(const bool value)
{
    m_isWaitingDisconnect = value;
}