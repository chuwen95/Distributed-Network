//
// Created by ChuWen on 2024/2/27.
//

#include "TcpSessionFactory.h"

#include "libcomponents/RingBuffer.h"

namespace service
{

    TcpSession::Ptr TcpSessionFactory::createTcpSession(const int fd, const std::size_t readBufferSize,
                                                        const std::size_t writeBufferSize)
    {
        components::RingBuffer::Ptr readBuffer = std::make_shared<components::RingBuffer>();
        readBuffer->init(readBufferSize);
        components::RingBuffer::Ptr writeBuffer = std::make_shared<components::RingBuffer>();
        writeBuffer->init(writeBufferSize);

        return std::make_shared<TcpSession>(fd, readBuffer, writeBuffer);
    }

} // service