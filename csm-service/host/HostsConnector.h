//
// Created by ChuWen on 9/25/23.
//

#ifndef TCPNETWORK_HOST_H
#define TCPNETWORK_HOST_H

#include "HostEndPointInfo.h"
#include "HostsInfoManager.h"
#include "csm-common/Common.h"
#include "csm-service//service/P2PSession.h"
#include "csm-utilities/Thread.h"

namespace csm
{

    namespace service
    {

        class HostsConnector
        {
        public:
            using Ptr = std::shared_ptr<HostsConnector>;

            HostsConnector(HostsInfoManager::Ptr hostsInfoManager);
            ~HostsConnector() = default;

        public:
            int init();

            int start();

            int stop();

            int registerConnectHandler(std::function<void(const int, P2PSession::Ptr)> connectHandler);

            int setHostConnected(const HostEndPointInfo& hostEndPointInfo);

            int setHostConnectedByFd(int fd);

        private:
            std::mutex x_connectingHosts;
            // ip:port => pair<fd, start_connect_timestamp>
            std::unordered_map<HostEndPointInfo, std::pair<int, std::int64_t>, HostEndPointInfo::hashFunction> m_connectingHosts;

            HostsInfoManager::Ptr m_hostsInfoManager;

            utilities::Thread::Ptr m_thread;

            std::function<void(const int, P2PSession::Ptr)> m_connectHandler;
        };

    } // namespace service

} // namespace csm

#endif // TCPNETWORK_HOST_H
