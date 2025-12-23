//
// Created by ChuWen on 9/28/23.
//

#include "HostsHeartbeatService.h"

#include "csm-service/protocol/header/PacketHeader.h"
#include "csm-service/protocol/payload/PayloadHeartBeat.h"
#include "csm-service/protocol/utilities/PacketEncodeHelper.h"
#include "csm-utilities/Logger.h"
#include "csm-utilities/TimeTools.h"

using namespace csm::service;

constexpr std::int32_t c_heartbeatInterval{5000};

HostsHeartbeatService::HostsHeartbeatService(const std::string& hostId, HostsInfoManager* hostsInfoManager)
    : m_hostId(hostId), m_hostInfoManager(hostsInfoManager)
{
}

int HostsHeartbeatService::init()
{
    const auto expression = [this]()
    {
        HostsInfoManager::Hosts hosts = m_hostInfoManager->getHosts();
        for (auto& host : hosts)
        {
            if (true == host.second.first.empty() || m_hostId == host.second.first)
            {
                continue;
            }

            std::uint64_t curTimestamp = utilities::TimeTools::getCurrentTimestamp();
            if (curTimestamp - host.second.second >= c_heartbeatInterval)
            {
                std::vector<char> buffer = PacketEncodeHelper<PacketType::PT_HeartBeat, std::nullopt_t>::encode();
                if (nullptr != m_heartBeatSender)
                {
                    SessionId sessionId;
                    if (0 != m_hostInfoManager->getSessionId(host.second.first, sessionId))
                    {
                        LOG->write(utilities::LogType::Log_Error, FILE_INFO, "get host fd failed, host: ", host.second.first);
                        return;
                    }
                    if (0 != m_heartBeatSender(sessionId, buffer))
                    {
                        LOG->write(utilities::LogType::Log_Error, FILE_INFO,
                                   "send heart beat failed, to host: ", host.second.first, ", session id: ", sessionId);
                    }
                    LOG->write(utilities::LogType::Log_Trace, FILE_INFO,
                               "send heart beat successfully, to host: ", host.second.first, ", session id: ", sessionId);

                    host.second.second = curTimestamp;
                }
            }
        }
    };
    m_thread = std::make_unique<utilities::Thread>(expression, c_heartbeatInterval, "host_heartbeat");

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

void HostsHeartbeatService::registerHeartbeatSender(std::function<int(const int, const std::vector<char>&)> heartbeatSender)
{
    m_heartBeatSender = std::move(heartbeatSender);
}