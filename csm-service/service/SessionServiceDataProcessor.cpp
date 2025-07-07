//
// Created by chu on 7/7/25.
//

#include "SessionServiceDataProcessor.h"

#include "csm-utilities/Logger.h"

using namespace csm::service;

SessionServiceDataProcessor::SessionServiceDataProcessor(utilities::Thread::Ptr thread) : m_thread(std::move(thread)) {}

int SessionServiceDataProcessor::init()
{
    const auto serviceDataProcess = [this]()
    {
        SessionServiceData sessionServiceData;
        m_sessionServiceDatas.wait_dequeue(sessionServiceData);

        auto iter = m_packetHandlers.find(sessionServiceData.header->type());
        if (m_packetHandlers.end() != iter)
        {
            if (0 != iter->second(sessionServiceData.fd, sessionServiceData.header, sessionServiceData.payload))
            {
                LOG->write(utilities::LogType::Log_Error, FILE_INFO, "process packet failed");
            }
        }
        else
        {
            LOG->write(utilities::LogType::Log_Error,
                FILE_INFO, "packet handler not register, type: ", static_cast<int>(sessionServiceData.header->type()));
        }
    };
    m_thread->setFunc(serviceDataProcess);
    m_thread->setInterval(1);
    m_thread->setName("serv_data_proc");

    return 0;
}

int SessionServiceDataProcessor::start()
{
    return m_thread->start();
}

int SessionServiceDataProcessor::stop()
{
    m_thread->stop();

    return 0;
}

int SessionServiceDataProcessor::addServiceDataPacket(const int fd, PacketHeader::Ptr header, PayloadBase::Ptr payload)
{
    return true == m_sessionServiceDatas.emplace(fd, header, payload) ? 0 : -1;
}

void SessionServiceDataProcessor::registerPacketHandler(const PacketType packetType, std::function<int(int fd, PacketHeader::Ptr header, PayloadBase::Ptr payload)> handler)
{
    m_packetHandlers[packetType] = std::move(handler);
}
