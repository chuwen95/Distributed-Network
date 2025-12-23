//
// Created by ChuWen on 9/25/23.
//

#ifndef TCPNETWORK_HOST_H
#define TCPNETWORK_HOST_H

#include "csm-common/Common.h"
#include "HostEndPointInfo.h"
#include "HostsInfoManager.h"
#include "csm-service/service/P2PSession.h"
#include "csm-service/service/P2PSessionFactory.h"
#include "csm-utilities/Thread.h"

namespace csm
{

    namespace service
    {

        class HostsConnector
        {
        public:
            explicit HostsConnector(HostsInfoManager* hostsInfoManager,
                                    std::shared_ptr<P2PSessionFactory> p2pSessionFactory);
            ~HostsConnector() = default;

        public:
            int init();

            int start();

            int stop();

            void registerConnectHandler(std::function<void(P2PSession::Ptr)> connectHandler);

            int setHostConnected(const HostEndPointInfo& hostEndPointInfo);

        private:
            std::mutex x_connectingHosts;
            // ip:port => pair<fd, start_connect_timestamp>
            std::unordered_map<HostEndPointInfo, std::pair<int, std::int64_t>, HostEndPointInfo::hashFunction> m_connectingHosts;

            HostsInfoManager* m_hostsInfoManager;
            std::shared_ptr<P2PSessionFactory> m_p2pSessionFactory;

            std::unique_ptr<utilities::Thread> m_thread;

            std::function<void(P2PSession::Ptr)> m_connectHandler;
        };

    } // namespace service

} // namespace csm

#endif // TCPNETWORK_HOST_H