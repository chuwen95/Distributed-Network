//
// Created by chu on 7/7/25.
//

#include "SessionServiceDataProcessor.h"

#include "csm-utilities/Logger.h"

using namespace csm::service;

int SessionServiceDataProcessor::init()
{
    const auto serviceDataProcess = [this](const std::stop_token& st)
    {
        if (true == st.stop_requested())
        {
            return;
        }

        SessionServiceData::Ptr sessionServiceData;
        m_sessionServiceDatas.wait_dequeue(sessionServiceData);

        if (true == st.stop_requested())
        {
            return;
        }


        LOG->write(utilities::LogType::Log_Debug, FILE_INFO,
                   "packet type: ", static_cast<int>(sessionServiceData->header->type()), ", session id: ",
                   sessionServiceData->sessionId);

        auto iter = m_packetHandlers.find(sessionServiceData->header->type());
        if (m_packetHandlers.end() != iter)
        {
            iter->second(sessionServiceData->sessionId, sessionServiceData->p2pSessionWeakPtr, sessionServiceData->header,
                         sessionServiceData->payload);
        }
        else
        {
            LOG->write(utilities::LogType::Log_Error, FILE_INFO,
                       "packet handler not register, type: ", static_cast<int>(sessionServiceData->header->type()));
        }
    };
    m_thread = std::make_unique<utilities::Thread>(serviceDataProcess, 1, "serv_data_proc");

    return 0;
}

int SessionServiceDataProcessor::start()
{
    m_thread->start();

    return 0;
}

int SessionServiceDataProcessor::stop()
{
    m_thread->stop();
    m_sessionServiceDatas.enqueue(nullptr);

    return 0;
}

int SessionServiceDataProcessor::addPacket(const SessionId sessionId, P2PSession::WPtr p2pSessionWeakPtr,
                                           PacketHeader::Ptr header, PayloadBase::Ptr payload)
{
    SessionServiceData::Ptr sessionServiceData = std::make_shared<SessionServiceData>(
        sessionId, std::move(p2pSessionWeakPtr), std::move(header), std::move(payload));
    return true == m_sessionServiceDatas.enqueue(sessionServiceData);
}

void SessionServiceDataProcessor::registerPacketHandler(
    const PacketType packetType,
    std::function<void(SessionId sessionId, P2PSession::WPtr p2pSessionWeakPtr, PacketHeader::Ptr header,
                       PayloadBase::Ptr payload)> handler)
{
    m_packetHandlers[packetType] = std::move(handler);
}