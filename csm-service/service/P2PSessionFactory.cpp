//
// Created by ChuWen on 2024/2/27.
//

#include "P2PSessionFactory.h"

#include "csm-utilities/RingBuffer.h"
#include "csm-utilities/UUIDTool.h"

using namespace csm::service;

P2PSession::Ptr P2PSessionFactory::create(const int fd, const std::size_t readBufferSize, const std::size_t writeBufferSize)
{
    std::unique_ptr<utilities::RingBuffer> readBuffer = std::make_unique<utilities::RingBuffer>(readBufferSize);
    readBuffer->init();
    std::unique_ptr<utilities::RingBuffer> writeBuffer = std::make_unique<utilities::RingBuffer>(writeBufferSize);
    writeBuffer->init();

    P2PSession::Ptr p2pSession = std::make_shared<P2PSession>(fd, m_sessionId, std::move(readBuffer), std::move(writeBuffer));
    ++m_sessionId;

    return p2pSession;
}