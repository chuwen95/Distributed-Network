//
// Created by ChuWen on 2024/8/22.
//

#include "SessionModuleDataProcessor.h"
#include "csm-utilities/Logger.h"

using namespace csm::service;

SessionModuleDataProcessor::SessionModuleDataProcessor(P2PSessionManager::Ptr p2pSessionManager, utilities::ThreadPool::Ptr normalPacketProcessor) :
    m_p2pSessionManager(std::move(p2pSessionManager)), m_normalPacketProcessor(std::move(normalPacketProcessor))
{}

int SessionModuleDataProcessor::init()
{
    return m_normalPacketProcessor->init();
}

int SessionModuleDataProcessor::start()
{
    return m_normalPacketProcessor->start();
}

int SessionModuleDataProcessor::stop()
{
    return m_normalPacketProcessor->stop();
}

void SessionModuleDataProcessor::addPackets(const int fd, PacketHeader::Ptr header, PayloadBase::Ptr payload)
{
    const auto packetProcess = [this, fd, capture_header = std::move(header), capture_payload = std::move(payload)]()
    {
        processPackets(fd, capture_header, capture_payload);
    };
    m_normalPacketProcessor->push(packetProcess);
}

void SessionModuleDataProcessor::registerPacketHandler(const PacketType packetType, std::function<int(const int fd, PacketHeader::Ptr header, PayloadBase::Ptr packet)> handler)
{
    m_packetHandler.emplace(packetType, std::move(handler));
}

int SessionModuleDataProcessor::processPackets(const int fd, PacketHeader::Ptr header, PayloadBase::Ptr payload)
{
    auto iter = m_packetHandler.find(header->type());
    if(m_packetHandler.end() != iter)
    {
        return iter->second(fd, header, payload);
    }

    return 0;
}