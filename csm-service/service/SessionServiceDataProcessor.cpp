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
        if (false == m_sessionServiceDatas.wait_dequeue_timed(sessionServiceData, 100000))
        {
            return;
        }

        if (false == m_running)
        {
            return;
        }

        LOG->write(utilities::LogType::Log_Debug, FILE_INFO,
                   "packet type: ", static_cast<int>(sessionServiceData.header->type()), ", fd: ", sessionServiceData.fd);
        auto iter = m_packetHandlers.find(sessionServiceData.header->type());
        if (m_packetHandlers.end() != iter)
        {
            if (0 != iter->second(sessionServiceData.fd, sessionServiceData.header, sessionServiceData.payload))
            {
                LOG->write(utilities::LogType::Log_Error, FILE_INFO,
                           "process packet failed, type: ", static_cast<int>(sessionServiceData.header->type()));
            }
        }
        else
        {
            LOG->write(utilities::LogType::Log_Error, FILE_INFO,
                       "packet handler not register, type: ", static_cast<int>(sessionServiceData.header->type()));
        }
    };
    m_thread->setFunc(serviceDataProcess);
    m_thread->setInterval(1);
    m_thread->setName("serv_data_proc");

    return 0;
}

int SessionServiceDataProcessor::start()
{
    if (true == m_running)
    {
        return 0;
    }

    m_running = true;
    m_thread->start();

    return 0;
}

int SessionServiceDataProcessor::stop()
{
    if (false == m_running)
    {
        return 0;
    }

    m_running = false;
    m_thread->stop();

    return 0;
}

int SessionServiceDataProcessor::addPacket(const int fd, PacketHeader::Ptr header, PayloadBase::Ptr payload)
{
    return true == m_sessionServiceDatas.enqueue(SessionServiceData(fd, header, payload)) ? 0 : -1;
}

void SessionServiceDataProcessor::registerPacketHandler(
    const PacketType packetType, std::function<int(int fd, PacketHeader::Ptr header, PayloadBase::Ptr payload)> handler)
{
    m_packetHandlers[packetType] = std::move(handler);
}
