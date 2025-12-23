//
// Created by ChuWen on 9/28/23.
//

#ifndef TCPNETWORK_HOSTSHEARTBEATSERVICE_H
#define TCPNETWORK_HOSTSHEARTBEATSERVICE_H

#include "HostsInfoManager.h"
#include "csm-service/service/SessionDispatcher.h"
#include "csm-utilities/Thread.h"

namespace csm
{

    namespace service
    {

        class HostsHeartbeatService
        {
        public:
            explicit HostsHeartbeatService(const std::string& hostId, HostsInfoManager* hostsInfoManager);
            ~HostsHeartbeatService() = default;

        public:
            int init();

            int start();

            int stop();

            void registerHeartbeatSender(std::function<int(const int fd, const std::vector<char>& data)>);

        private:
            std::string m_hostId;
            HostsInfoManager* m_hostInfoManager;
            std::function<int(const int fd, const std::vector<char>& data)> m_heartBeatSender;

            std::unique_ptr<utilities::Thread> m_thread;
        };

    } // namespace service

} // namespace csm

#endif // TCPNETWORK_HOSTSHEARTBEATSERVICE_H
