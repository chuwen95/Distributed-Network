//
// Created by ChuWen on 9/20/23.
//

#ifndef TCPNETWORK_SERVICECONFIG_H
#define TCPNETWORK_SERVICECONFIG_H

#include "csm-common/Common.h"
#include "inipp.h"
#include "csm-utilities/Logger.h"
#include "csm-tool/NodeConfig.h"

#include "csm-utilities/SelectListenner.h"
#include "csm-service/service/Acceptor.h"
#include "csm-service/service/SessionDispatcher.h"
#include "csm-service/service/SessionDestroyer.h"
#include "csm-service/service/SessionDataProcessor.h"
#include "csm-utilities/ThreadPool.h"

#include "csm-service/host/HostsInfoManager.h"
#include "csm-service/host/HostsConnector.h"
#include "csm-service/host/HostsHeartbeatService.h"

#include "csm-service/service/TcpSessionManager.h"
#include "csm-service/service/ClientAliveChecker.h"

namespace csm
{

    namespace service
    {

        class ServiceConfig
        {
        public:
            using Ptr = std::shared_ptr<ServiceConfig>;

            ServiceConfig(tool::NodeConfig::Ptr nodeConfig, utilities::SelectListenner::Ptr listenner, Acceptor::Ptr acceptor,
                          TcpSessionManager::Ptr tcpSessionManager, ClientAliveChecker::Ptr clientAliveChecker, std::vector<SlaveReactor::Ptr> slaveReactors,
                          SessionDispatcher::Ptr tcpSessionDispatcher, SessionDestroyer::Ptr sessionDestroyer, SessionDataProcessor::Ptr sessionDataProcessor,
                          HostsInfoManager::Ptr hostsInfoManager, HostsConnector::Ptr hostsConnector, HostsHeartbeatService::Ptr hostsHeartbeatService);

            ~ServiceConfig() = default;

        public:
            tool::NodeConfig::Ptr nodeConfig();

            utilities::SelectListenner::Ptr listenner();
            Acceptor::Ptr acceptor();

            TcpSessionManager::Ptr tcpSessionManager();
            ClientAliveChecker::Ptr clientAliveChecker();

            std::vector<SlaveReactor::Ptr>& slaveReactors();
            SessionDispatcher::Ptr sessionDispatcher();
            SessionDestroyer::Ptr sessionDestroyer();
            SessionDataProcessor::Ptr sessionDataProcessor();

            HostsInfoManager::Ptr hostsInfoManager();
            HostsConnector::Ptr hostsConnector();
            HostsHeartbeatService::Ptr hostsHeartbeatService();

        private:
            tool::NodeConfig::Ptr m_nodeConfig;

            utilities::SelectListenner::Ptr m_listenner;
            Acceptor::Ptr m_acceptor;

            TcpSessionManager::Ptr m_tcpSessionManager;
            ClientAliveChecker::Ptr m_clientAliveChecker;
            std::vector<SlaveReactor::Ptr> m_slaveReactors;

            SessionDispatcher::Ptr m_sessionDispatcher;
            SessionDestroyer::Ptr m_sessionDestroyer;
            SessionDataProcessor::Ptr m_sessionDataProcessor;

            utilities::ThreadPool::Ptr m_packetProcesser;

            HostsInfoManager::Ptr m_hostsInfoManager;
            HostsConnector::Ptr m_hostsConnector;
            HostsHeartbeatService::Ptr m_hostsHeartbeatService;
        };

    } // server

}

#endif //TCPNETWORK_SERVICECONFIG_H
