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
            using Ptr = std::shared_ptr<HostsHeartbeatService>;

            HostsHeartbeatService(const std::string &hostId, HostsInfoManager::Ptr hostsInfoManager);
            ~HostsHeartbeatService() = default;

        public:
            int init();

            int start();

            int stop();

            void registerHeartbeatSender(std::function<int(const int fd, const std::vector<char> &data)>);

        private:
            std::string m_hostId;
            HostsInfoManager::Ptr m_hostInfoManager;
            std::function<int(const int fd, const std::vector<char> &data)> m_heartBeatSender;

            utilities::Thread::Ptr m_thread;
        };

    } // service

}

#endif //TCPNETWORK_HOSTSHEARTBEATSERVICE_H
