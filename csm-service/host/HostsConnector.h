//
// Created by root on 9/25/23.
//

#ifndef TCPNETWORK_HOST_H
#define TCPNETWORK_HOST_H

#include "csm-common/Common.h"
#include "csm-utilities/Thread.h"
#include "csm-service//service/TcpSession.h"
#include "HostEndPointInfo.h"
#include "HostsInfoManager.h"

namespace csm
{

    namespace service
    {

        class HostsConnector
        {
        public:
            using Ptr = std::shared_ptr<HostsConnector>;

        public:
            int init(HostsInfoManager::Ptr hostsInfoManager);

            int uninit();

            int start();

            int stop();

            int registerConnectHandler(std::function<void(const int, TcpSession::Ptr)> connectHandler);

            int setHostConnected(const HostEndPointInfo &hostEndPointInfo);

        private:
            std::mutex x_connectingHosts;
            // ip:port => pair<fd, start_connect_timestamp>
            std::unordered_map<HostEndPointInfo, std::pair<int, std::int64_t>, HostEndPointInfo::hashFunction> m_connectingHosts;

            HostsInfoManager::Ptr m_hostsInfoManager;

            utilities::Thread m_thread;

            std::function<void(const int, TcpSession::Ptr)> m_connectHandler;
        };

    } // service

}

#endif //TCPNETWORK_HOST_H
