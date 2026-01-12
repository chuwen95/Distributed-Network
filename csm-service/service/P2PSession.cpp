//
// Created by ChuWen on 9/5/23.
//

#include "P2PSession.h"

#include "csm-utilities/Socket.h"

using namespace csm::service;

P2PSession::P2PSession(const int fd, const SessionId sessionId, std::unique_ptr<utilities::RingBuffer> readBuffer,
                       std::unique_ptr<utilities::RingBuffer> writeBuffer)
    : m_fd(fd), m_sessionId(sessionId), m_readBuffer(std::move(readBuffer)), m_writeBuffer(std::move(writeBuffer))
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

SessionId P2PSession::sessionId() const
{
    return m_sessionId;
}

std::mutex& P2PSession::readBufferMutex()
{
    return x_readBuffer;
}

csm::utilities::RingBuffer* P2PSession::readBuffer()
{
    return m_readBuffer.get();
}

std::mutex& P2PSession::writeBufferMutex()
{
    return x_writeBuffer;
}

csm::utilities::RingBuffer* P2PSession::writeBuffer()
{
    return m_writeBuffer.get();
}

void P2PSession::setNodeId(std::string id)
{
    m_nodeId = std::move(id);
}

std::string P2PSession::nodeId() const
{
    return m_nodeId;
}

void P2PSession::setPeerHostEndPointInfo(const service::HostEndPointInfo& hostEndPointInfo)
{
    m_peerHostEndPointInfo = hostEndPointInfo;
}

const HostEndPointInfo& P2PSession::peerHostEndPointInfo() const
{
    return m_peerHostEndPointInfo;
}

bool P2PSession::isWaitingDisconnect() const
{
    return m_isWaitingDisconnect;
}

void P2PSession::setWaitingDisconnect(const bool value)
{
    m_isWaitingDisconnect = value;
}

void P2PSession::setSlaveReactorIndex(std::size_t index)
{
    m_slaveReactorIndex = index;
}

std::size_t P2PSession::slaveReactorIndex() const
{
    return m_slaveReactorIndex;
}

bool P2PSession::isHeartbeatTimeout()
{
    bool ret = (m_lastRecvCount == m_recvCount);
    m_lastRecvCount = m_recvCount;

    return ret;
}

void P2PSession::refreshHeartbeat()
{
    m_recvCount += 1;
}
