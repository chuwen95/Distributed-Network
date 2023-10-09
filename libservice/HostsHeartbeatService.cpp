//
// Created by root on 9/28/23.
//

#include "HostsHeartbeatService.h"

#include "libcomponents/CellTimestamp.h"
#include "libpacketprocess/packet/PacketHeartBeat.h"

namespace service
{

    constexpr std::int32_t c_heartbeatInterval{5000};

    int HostsHeartbeatService::init(const std::string& hostId, HostsInfoManager::Ptr hostsInfoManager)
    {
        m_hostId = hostId;
        m_hostInfoManager = hostsInfoManager;

        const auto expression = [this]()
        {
            HostsInfoManager::Hosts hosts = m_hostInfoManager->getHosts();
            for(auto& host : hosts)
            {
                if(true == host.second.first.empty() || m_hostId == host.second.first)
                {
                    continue;
                }

                std::int64_t curTimestamp = components::CellTimestamp::getCurrentTimestamp();
                if(curTimestamp - host.second.second >= c_heartbeatInterval)
                {
                    packetprocess::PacketHeartBeat packetHeartbeat;
                    packetHeartbeat.setId(m_hostId);
                    packetHeartbeat.setTimestamp(curTimestamp);
                    int payloadSize = packetHeartbeat.packetLength();

                    packetprocess::PacketHeader packetHeader;
                    packetHeader.setType(packetprocess::PacketType::PT_HeartBeat);
                    packetHeader.setPayloadLength(payloadSize);
                    int headerSize = packetHeader.headerLength();

                    std::vector<char> buffer;
                    buffer.resize(headerSize + payloadSize);
                    packetHeader.encode(buffer.data(), headerSize);
                    packetHeartbeat.encode(buffer.data() + headerSize, payloadSize);

                    if(nullptr != m_heartBeatSender)
                    {
                        m_heartBeatSender(m_hostInfoManager->getHostFdById(host.second.first), buffer);
                        host.second.second = curTimestamp;
                    }
                }
            }
        };
        m_thread.init(expression, 200, "host_heartbeat");

        return 0;
    }

    int HostsHeartbeatService::uninit()
    {
        m_thread.uninit();

        return 0;
    }

    int HostsHeartbeatService::start()
    {
        m_thread.start();

        return 0;
    }

    int HostsHeartbeatService::stop()
    {
        m_thread.stop();

        return 0;
    }

    void HostsHeartbeatService::registerHeartbeatSender(std::function<int(const int, const std::vector<char> &)> heartbeatSender)
    {
        m_heartBeatSender = std::move(heartbeatSender);
    }

} // service