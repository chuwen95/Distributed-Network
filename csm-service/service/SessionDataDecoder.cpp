//
// Created by chu on 7/3/25.
//

#include "SessionDataDecoder.h"

#include <cstring>

#include "csm-utilities/Logger.h"

using namespace csm::service;

SessionDataDecoder::SessionDataDecoder(PayloadFactory::Ptr payloadFactory, utilities::ThreadPool::Ptr decodeWorkers)
    : m_payloadFactory(std::move(payloadFactory)), m_decodeWorkers(std::move(decodeWorkers))
{
}

int SessionDataDecoder::start()
{
    if (true == m_running)
    {
        return 0;
    }

    m_running = true;
    return m_decodeWorkers->start();
}

int SessionDataDecoder::stop()
{
    if (false == m_running)
    {
        return 0;
    }

    m_running = false;
    return m_decodeWorkers->stop();
}

void SessionDataDecoder::addSessionData(const SessionId sessionId, P2PSession::WPtr p2pSessionWeakPtr, const char* data,
                                        const std::size_t dataLen)
{
    std::shared_ptr<std::vector<char>> buffer = std::make_shared<std::vector<char>>(data, data + dataLen);
    memcpy(buffer->data(), data, dataLen);

    const auto decode = [this, sessionId, captureSession = std::move(p2pSessionWeakPtr), captureBuffer =
            std::move(buffer)]()
    {
        P2PSession::Ptr p2pSession = captureSession.lock();
        if (nullptr == p2pSession)
        {
            LOG->write(utilities::LogType::Log_Error, FILE_INFO, "session may disconnected and destoryed, session id: ",
                       sessionId);
            return;
        }

        if (true == p2pSession->isWaitingDisconnect())
        {
            return;
        }

        std::unique_lock<std::mutex> ulock(p2pSession->readBufferMutex());

        // 将数据写入到缓冲区尾部
        utilities::RingBuffer* readBuffer = p2pSession->readBuffer();
        if (-1 == writeDataToP2PSessionReadBuffer(p2pSession->sessionId(), readBuffer, captureBuffer))
        {
            LOG->write(utilities::LogType::Log_Error, FILE_INFO, "write data to buffer failed, session id: ",
                       p2pSession->sessionId(),
                       ", data size: ", captureBuffer->size());
            return;
        }

        while (true == m_running)
        {
            PacketHeader::Ptr header = decodePacketHeader(readBuffer);
            if (nullptr == header)
            {
                break;
            }

            PayloadBase::Ptr payload{nullptr};
            if (0 != header->payloadLength())
            {
                payload = decodePacketPayload(header, readBuffer);
                if (nullptr == payload)
                {
                    break;
                }
            }

            readBuffer->decreaseUsedSpace(header->headerLength() + header->payloadLength());

            if (nullptr != m_packetHandler)
            {
                LOG->write(utilities::LogType::Log_Debug, FILE_INFO, "received packet from fd: ", p2pSession->fd(),
                           ", packet type: ", header->type());
                m_packetHandler(p2pSession->sessionId(), captureSession, header, payload);
            }
        }
    };
    m_decodeWorkers->push(decode);
}

void SessionDataDecoder::setPacketHandler(
    std::function<void(SessionId sessionId, P2PSession::WPtr p2pSessionWeakPtr, PacketHeader::Ptr header,
                       PayloadBase::Ptr payload)> handler)
{
    m_packetHandler = std::move(handler);
}

int SessionDataDecoder::writeDataToP2PSessionReadBuffer(SessionId sessionId, utilities::RingBuffer* readBuffer,
                                                        const std::shared_ptr<std::vector<char>>& buffer)
{
    int ret = readBuffer->writeData(buffer->data(), buffer->size());
    if (-1 == ret)
    {
        return -1;
    }

    // 增加已经使用的空间
    ret = readBuffer->increaseUsedSpace(buffer->size());
    if (-1 == ret)
    {
        LOG->write(utilities::LogType::Log_Error, FILE_INFO, "increaseUsedSpace failed, space: ", readBuffer->space(),
                   ", data len: ", buffer->size(), ", sessionId: ", sessionId);
        assert(-1 != ret);
    }

    return 0;
}

PacketHeader::Ptr SessionDataDecoder::decodePacketHeader(utilities::RingBuffer* readBuffer)
{
    PacketHeader::Ptr header = std::make_shared<PacketHeader>();
    const std::size_t headerLength = header->headerLength();

    LOG->write(utilities::LogType::Log_Debug, FILE_INFO, "readBuffer->length(): ", readBuffer->dataLength());

    char* buffer{nullptr};
    int ret = readBuffer->getBufferForRead(headerLength, buffer);
    if (-1 == ret) // 缓冲区数据量不足
    {
        LOG->write(utilities::LogType::Log_Debug, FILE_INFO, "buffer data length less than header length");
        return nullptr;
    }
    else if (-2 == ret)
    {
        std::vector<char> bufferForBackspaceLessThanHeaderLength;
        bufferForBackspaceLessThanHeaderLength.resize(headerLength);
        readBuffer->readData(headerLength, 0, bufferForBackspaceLessThanHeaderLength);

        buffer = bufferForBackspaceLessThanHeaderLength.data();
    }

    header->decode(buffer, headerLength);
    assert(true == header->isMagicMatch());

    LOG->write(utilities::LogType::Log_Debug, FILE_INFO,
               "decode payload header successfully, payload type: ", static_cast<int>(header->type()),
               ", payload length: ", header->payloadLength());

    return header;
}

PayloadBase::Ptr SessionDataDecoder::decodePacketPayload(const PacketHeader::Ptr& header,
                                                         utilities::RingBuffer* readBuffer)
{
    // 尝试复制包数据
    std::size_t headerLength = header->headerLength();
    std::size_t payloadLength = header->payloadLength();
    if (readBuffer->dataLength() - headerLength < payloadLength)
    {
        LOG->write(utilities::LogType::Log_Trace, FILE_INFO,
                   "payload length not enough, payload type: ", static_cast<int>(header->type()));
        return nullptr;
    }

    char* buffer{nullptr};
    std::size_t continuousDataLen{0};
    int ret = readBuffer->getContinuousData(buffer, continuousDataLen);
    assert(-1 != ret);

    PayloadBase::Ptr payload;
    if (continuousDataLen < headerLength + payloadLength)
    {
        // 连续数据不够headerLength + payloadLength
        std::vector<char> payloadData;
        payloadData.resize(payloadLength);
        if (-1 == (ret = readBuffer->readData(payloadLength, headerLength, payloadData)))
        {
            LOG->write(utilities::LogType::Log_Trace, FILE_INFO,
                       "payload length not enough, payload type: ", static_cast<int>(header->type()));
            assert(-1 != ret);
        }

        payload = m_payloadFactory->createPayload(header->type(), payloadData.data(), payloadLength);
    }
    else
    {
        payload = m_payloadFactory->createPayload(header->type(), buffer + headerLength, payloadLength);
    }

    return payload;
}
