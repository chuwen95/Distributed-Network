//
// Created by ChuWen on 2024/8/22.
//

#include "SessionDataProcessor.h"
#include "csm-utilities/Logger.h"

using namespace csm::service;

SessionDataProcessor::SessionDataProcessor(TcpSessionManager::Ptr tcpSessionManager, ClientAliveChecker::Ptr clientAliveChecker, const std::size_t workerNum) :
        m_tcpSessionManager(std::move(tcpSessionManager)),
        m_clientAliveChecker(std::move(clientAliveChecker)),
        m_worker(std::make_shared<utilities::ThreadPool>(workerNum, "sess_dt_work"))
{}

int SessionDataProcessor::init()
{
    return m_worker->init();
}

int SessionDataProcessor::start()
{
    return m_worker->start();
}

int SessionDataProcessor::stop()
{
    return m_worker->stop();
}

int SessionDataProcessor::addSessionData(const int fd, const char* data, const std::size_t dataLen)
{
    std::shared_ptr<std::vector<char>> buffer = std::make_shared<std::vector<char>>();
    buffer->resize(dataLen);
    memcpy(buffer->data(), data, dataLen);

    const auto expression = [fd, buffer, this]() {
        TcpSession::Ptr tcpSession = m_tcpSessionManager->tcpSession(fd);
        if(nullptr == tcpSession)
        {
            LOG->write(utilities::LogType::Log_Error, FILE_INFO, "find TcpSession failed, fd: ", fd);
            return;
        }

        if(true == tcpSession->isWaitingDisconnect())
        {
            return;
        }

        std::vector<std::pair<PacketHeader::Ptr, PayloadBase::Ptr>> packets;

        {
            std::unique_lock<std::mutex> ulock(tcpSession->readBufferMutex());

            utilities::RingBuffer::Ptr readBuffer = tcpSession->readBuffer();
            if (-1 == writeDataToTcpSessionReadBuffer(fd, readBuffer, buffer))
            {
                LOG->write(utilities::LogType::Log_Error, FILE_INFO, "write data to buffer failed, fd: ", fd, ", data size: ", buffer->size());
                return;
            }

            decodePackets(readBuffer, packets);
        }

        processPackets(fd, packets);
    };
    m_worker->push(expression);

    return 0;
}

void SessionDataProcessor::registerPacketHandler(const PacketType packetType, std::function<int(const int fd, PacketHeader::Ptr header, PayloadBase::Ptr packet)> handler)
{
    m_packetHandler.emplace(packetType, std::move(handler));
}

int SessionDataProcessor::writeDataToTcpSessionReadBuffer(const int fd, csm::utilities::RingBuffer::Ptr readBuffer, std::shared_ptr<std::vector<char>> buffer)
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

int SessionDataProcessor::processPackets(const int fd, std::vector<std::pair<PacketHeader::Ptr, PayloadBase::Ptr>>& packets)
{
    for(std::pair<PacketHeader::Ptr, PayloadBase::Ptr>& packet : packets)
    {
        switch(packet.first->type())
        {
            case PacketType::PT_None:
                LOG->write(utilities::LogType::Log_Error, FILE_INFO, "payload type is none, fd: ", fd);
                break;
            case PacketType::PT_ClientInfo:     // ServiceStartType为Server的时候，这个case不会走到
            case PacketType::PT_ClientInfoReply:     // ServiceStartType为Server的时候，这个case不会走到
            case PacketType::PT_ModuleMessage:
            {
                m_clientAliveChecker->refreshClientLastRecvTime(fd);

                auto iter = m_packetHandler.find(packet.first->type());
                if(m_packetHandler.end() != iter)
                {
                    iter->second(fd, packet.first, packet.second);
                }
                break;
            }
            case PacketType::PT_HeartBeat:
            case PacketType::PT_HeartBeatReply:
                m_clientAliveChecker->refreshClientLastRecvTime(fd);

                LOG->write(utilities::LogType::Log_Trace, FILE_INFO, "receive heartbeat payload, fd: ", fd);
                break;
            default:
                LOG->write(utilities::LogType::Log_Error, FILE_INFO, "unknown payload type, fd: ", fd);
                break;
        }
    }
    return 0;
}

int SessionDataProcessor::decodePackets(utilities::RingBuffer::Ptr &readBuffer, std::vector<std::pair<PacketHeader::Ptr, PayloadBase::Ptr>>& packets)
{
    while(true)
    {
        PacketHeader::Ptr header = std::make_shared<PacketHeader>();
        const std::size_t headerLength = header->headerLength();

        LOG->write(utilities::LogType::Log_Debug, FILE_INFO, "readBuffer->length(): ", readBuffer->dataLength());
        if (readBuffer->dataLength() < headerLength)
        {
            LOG->write(utilities::LogType::Log_Debug, FILE_INFO, "buffer data length less than header length");
            break;
        }

        char *buffer{nullptr};
        std::vector<char> bufferForBackspaceLessThanHeaderLength;

        int ret = readBuffer->getBufferForRead(headerLength, buffer);
        assert(-1 != ret);
        if (-2 == ret)
        {
            bufferForBackspaceLessThanHeaderLength.resize(headerLength);
            readBuffer->readData(headerLength, 0, bufferForBackspaceLessThanHeaderLength);
            buffer = bufferForBackspaceLessThanHeaderLength.data();
        }

        header->decode(buffer, headerLength);
        assert(true == header->isMagicMatch());

        LOG->write(utilities::LogType::Log_Debug, FILE_INFO,
                   "decode payload header successfully, payload type: ", static_cast<int>(header->type()), ", payload length: ", header->payloadLength());

        // 尝试复制包数据
        std::size_t payloadLength = header->payloadLength();
        if (readBuffer->dataLength() - headerLength < payloadLength)
        {
            LOG->write(utilities::LogType::Log_Trace, FILE_INFO, "payload length not enough, payload type: ", static_cast<int>(header->type()));
            break;
        }

        std::size_t continuousDataLen{0};
        ret = readBuffer->getContinuousData(buffer, continuousDataLen);
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

            payload = PayloadFactory().createPayload(header->type(), payloadData.data(), payloadLength);
        }
        else
        {
            payload = PayloadFactory().createPayload(header->type(), buffer + headerLength, payloadLength);
        }

        readBuffer->decreaseUsedSpace(headerLength + payloadLength);

        packets.emplace_back(header, payload);
    }

    return 0;
}
