//
// Created by ChuWen on 2024/8/22.
//

#include "SessionModuleDataProcessor.h"
#include "csm-utilities/Logger.h"

using namespace csm::service;

SessionModuleDataProcessor::SessionModuleDataProcessor(utilities::ThreadPool::Ptr normalPacketProcessor)
    : m_normalPacketProcessor(std::move(normalPacketProcessor))
{
}

int SessionModuleDataProcessor::start()
{
    return m_normalPacketProcessor->start();
}

int SessionModuleDataProcessor::stop()
{
    return m_normalPacketProcessor->stop();
}

void SessionModuleDataProcessor::addPacket(const SessionId sessionId, P2PSession::WPtr p2pSessionWeakPtr, PacketHeader::Ptr header, PayloadBase::Ptr payload)
{
    const auto packetProcess = [this, sessionId, captureSession = std::move(p2pSessionWeakPtr), captureHeader = std::move(header), capturePayload = std::move(payload)]()
    {
        auto iter = m_packetHandler.find(captureHeader->type());
        if (m_packetHandler.end() == iter)
        {
            LOG->write(utilities::LogType::Log_Error, FILE_INFO, "find packet type handler failed, type: {}", captureHeader->type());
            return;
        }

        iter->second(sessionId, captureSession, captureHeader, capturePayload);
    };
    m_normalPacketProcessor->push(packetProcess);
}

void SessionModuleDataProcessor::registerPacketHandler(
    const PacketType packetType, std::function<void(const SessionId sessionId, P2PSession::WPtr p2pSessionWeakPtr, PacketHeader::Ptr header, PayloadBase::Ptr packet)> handler)
{
    m_packetHandler.emplace(packetType, std::move(handler));
}