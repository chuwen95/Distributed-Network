//
// Created by chu on 7/3/25.
//

#include "SessionDataDecoder.h"

#include "csm-packetprocess/concept/PacketConcept.h"
#include "csm-utilities/Logger.h"

using namespace csm::service;

SessionDataDecoder::SessionDataDecoder(
    P2PSessionManager::Ptr p2pSessionManager, PayloadFactory::Ptr payloadFactory, utilities::ThreadPool::Ptr decodeWorkers) :
    m_p2pSessionManager(std::move(p2pSessionManager)), m_payloadFactory(std::move(payloadFactory)), m_decodeWorkers(std::move(decodeWorkers))
{
}

int SessionDataDecoder::init()
{
    return m_decodeWorkers->init();
}

int SessionDataDecoder::start()
{
    return m_decodeWorkers->start();
}

int SessionDataDecoder::stop()
{
    return m_decodeWorkers->stop();
}

int SessionDataDecoder::addSessionData(const int fd, const char* data, const std::size_t dataLen)
{
    std::shared_ptr<std::vector<char>> buffer = std::make_shared<std::vector<char>>(data, data + dataLen);
    memcpy(buffer->data(), data, dataLen);

    const auto decode = [this, fd, captureBuffer = std::move(buffer)]()
    {
        P2PSession::Ptr p2pSession = m_p2pSessionManager->session(fd);
        if(nullptr == p2pSession)
        {
            LOG->write(utilities::LogType::Log_Error, FILE_INFO, "find P2PSession failed, fd: ", fd);
            return;
        }

        if(true == p2pSession->isWaitingDisconnect())
        {
            return;
        }

        while (true)
        {
            std::unique_lock<std::mutex> ulock(p2pSession->readBufferMutex());

            // 将数据写入到缓冲区尾部
            utilities::RingBuffer::Ptr& readBuffer = p2pSession->readBuffer();
            if (-1 == writeDataToP2PSessionReadBuffer(fd, readBuffer, captureBuffer))
            {
                LOG->write(utilities::LogType::Log_Error, FILE_INFO, "write data to buffer failed, fd: ", fd, ", data size: ", captureBuffer->size());
                return;
            }

            PacketHeader::Ptr header = decodePacketHeader(fd, readBuffer);
            if (nullptr == header)
            {
                return;
            }

            PayloadBase::Ptr payload = decodePacketPayload(fd, header, readBuffer);
            if (nullptr == payload)
            {
                return;
            }

            if (nullptr != m_packetHandler)
            {
                m_packetHandler(fd, header, payload);
            }
        }
    };
    m_decodeWorkers->push(decode);

    return 0;
}

int SessionDataDecoder::writeDataToP2PSessionReadBuffer(
    const int fd, const utilities::RingBuffer::Ptr& readBuffer, const std::shared_ptr<std::vector<char>>& buffer)
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
        LOG->write(utilities::LogType::Log_Error, FILE_INFO, "increaseUsedSpace failed, space: ", readBuffer->space(), ", data len: ", buffer->size(), ", fd: ", fd);
        assert(-1 != ret);
    }

    return 0;
}

PacketHeader::Ptr SessionDataDecoder::decodePacketHeader(const int fd, const utilities::RingBuffer::Ptr& readBuffer)
{
    PacketHeader::Ptr header = std::make_shared<PacketHeader>();
    const std::size_t headerLength = header->headerLength();

    LOG->write(utilities::LogType::Log_Debug, FILE_INFO, "readBuffer->length(): ", readBuffer->dataLength());

    char *buffer{nullptr};
    int ret = readBuffer->getBufferForRead(headerLength, buffer);
    if (-1 == ret)  // 缓冲区数据量不足
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
               "decode payload header successfully, payload type: ", static_cast<int>(header->type()), ", payload length: ", header->payloadLength());

    return header;
}

PayloadBase::Ptr SessionDataDecoder::decodePacketPayload(const int fd, const PacketHeader::Ptr& header, const utilities::RingBuffer::Ptr& readBuffer)
{
    // 尝试复制包数据
    std::size_t headerLength = header->headerLength();
    std::size_t payloadLength = header->payloadLength();
    if (readBuffer->dataLength() - headerLength < payloadLength)
    {
        LOG->write(utilities::LogType::Log_Trace, FILE_INFO, "payload length not enough, payload type: ", static_cast<int>(header->type()));
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
            LOG->write(utilities::LogType::Log_Trace, FILE_INFO, "payload length not enough, payload type: ", static_cast<int>(header->type()));
            assert(-1 != ret);
        }

        payload = m_payloadFactory->createPayload(header->type(), payloadData.data(), payloadLength);
    }
    else
    {
        payload = m_payloadFactory->createPayload(header->type(), buffer + headerLength, payloadLength);
    }

    readBuffer->decreaseUsedSpace(headerLength + payloadLength);

    return payload;
}
