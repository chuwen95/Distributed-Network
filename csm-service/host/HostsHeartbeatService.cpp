//
// Created by ChuWen on 9/28/23.
//

#include "HostsHeartbeatService.h"

#include "csm-utilities/TimeTools.h"
#include "csm-utilities/Logger.h"
#include "csm-service/protocol/PacketHeader.h"
#include "csm-service/protocol/payload/PayloadHeartBeat.h"

using namespace csm::service;

constexpr std::int32_t c_heartbeatInterval{5000};

HostsHeartbeatService::HostsHeartbeatService(const std::string &hostId, HostsInfoManager::Ptr hostsInfoManager) :
    m_hostId(hostId), m_hostInfoManager(std::move(hostsInfoManager))
{
}

int HostsHeartbeatService::init()
{
    const auto expression = [this]() {
        HostsInfoManager::Hosts hosts = m_hostInfoManager->getHosts();
        for (auto &host: hosts)
        {
            if (true == host.second.first.empty() || m_hostId == host.second.first)
            {
                continue;
            }

            std::int64_t curTimestamp = utilities::TimeTools::getCurrentTimestamp();
            if (curTimestamp - host.second.second >= c_heartbeatInterval)
            {
                PayloadHeartBeat packetHeartbeat;
                packetHeartbeat.setNodeId(m_hostId);
                packetHeartbeat.setTimestamp(curTimestamp);
                int payloadSize = packetHeartbeat.packetLength();

                PacketHeader packetHeader;
                packetHeader.setType(PacketType::PT_HeartBeat);
                packetHeader.setPayloadLength(payloadSize);
                int headerSize = packetHeader.headerLength();

                std::vector<char> buffer;
                buffer.resize(headerSize + payloadSize);
                packetHeader.encode(buffer.data(), headerSize);
                packetHeartbeat.encode(buffer.data() + headerSize, payloadSize);

                if (nullptr != m_heartBeatSender)
                {
                    int fd{-1};
                    if (0 != m_hostInfoManager->getHostFdById(host.second.first, fd))
                    {
                        LOG->write(utilities::LogType::Log_Error, FILE_INFO, "get host fd failed, host: ", host.second.first);
                        return;
                    }
                    if (0 != m_heartBeatSender(fd, buffer))
                    {
                        LOG->write(utilities::LogType::Log_Error, FILE_INFO, "send heart beat failed, to host: ", host.second.first, ", fd: ", fd);
                        return;
                    }
                    host.second.second = curTimestamp;
                }
            }
        }
    };
    m_thread = std::make_shared<utilities::Thread>();
    m_thread->setFunc(expression);
    m_thread->setInterval(c_heartbeatInterval);
    m_thread->setName("host_heartbeat");

    return 0;
}

int HostsHeartbeatService::start()
{
    m_thread->start();

    return 0;
}

int HostsHeartbeatService::stop()
{
    m_thread->stop();

    return 0;
}

void HostsHeartbeatService::registerHeartbeatSender(std::function<int(const int, const std::vector<char> &)> heartbeatSender)
{
    m_heartBeatSender = std::move(heartbeatSender);
}