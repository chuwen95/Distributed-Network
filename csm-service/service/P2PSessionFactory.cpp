//
// Created by ChuWen on 2024/2/27.
//

#include "P2PSessionFactory.h"

#include "csm-utilities/RingBuffer.h"

using namespace csm::service;

P2PSession::Ptr P2PSessionFactory::create(const int fd, const std::size_t readBufferSize, const std::size_t writeBufferSize)
{
    utilities::RingBuffer::Ptr readBuffer = std::make_shared<utilities::RingBuffer>(readBufferSize);
    readBuffer->init();
    utilities::RingBuffer::Ptr writeBuffer = std::make_shared<utilities::RingBuffer>(writeBufferSize);
    writeBuffer->init();

    return std::make_shared<P2PSession>(fd, readBuffer, writeBuffer);
}