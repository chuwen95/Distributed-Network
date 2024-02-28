//
// Created by root on 9/20/23.
//

#ifndef TCPNETWORK_SERVICECONFIG_H
#define TCPNETWORK_SERVICECONFIG_H

#include "libcommon/Common.h"
#include "inipp.h"
#include "libcomponents/Logger.h"
#include "libtools/NodeConfig.h"

#include "libcomponents/SelectListenner.h"
#include "../service/Acceptor.h"
#include "../service/SlaveReactorManager.h"
#include "libcomponents/ThreadPool.h"

#include "../host/HostsInfoManager.h"
#include "../host/HostsConnector.h"
#include "../host/HostsHeartbeatService.h"

namespace service
{

    class ServiceConfig
    {
    public:
        using Ptr = std::shared_ptr<ServiceConfig>;

        ServiceConfig(tools::NodeConfig::Ptr nodeConfig, components::SelectListenner::Ptr listenner, Acceptor::Ptr acceptor,
                      std::vector<SlaveReactor::Ptr> slaveReactors, SlaveReactorManager::Ptr slaveReactorManager, components::ThreadPool::Ptr packetProcessor,
                      HostsInfoManager::Ptr hostsInfoManager, HostsConnector::Ptr hostsConnector, HostsHeartbeatService::Ptr hostsHeartbeatService);
        ~ServiceConfig() = default;

    public:
        tools::NodeConfig::Ptr nodeConfig();

        components::SelectListenner::Ptr listenner();
        Acceptor::Ptr acceptor();
        std::vector<SlaveReactor::Ptr>& slaveReactors();
        SlaveReactorManager::Ptr slaveReactorManager();
        components::ThreadPool::Ptr packetProcessor();

        HostsInfoManager::Ptr hostsInfoManager();
        HostsConnector::Ptr hostsConnector();
        HostsHeartbeatService::Ptr hostsHeartbeatService();

    private:
        tools::NodeConfig::Ptr m_nodeConfig;

        components::SelectListenner::Ptr m_listenner;
        Acceptor::Ptr m_acceptor;
        std::vector<SlaveReactor::Ptr> m_slaveReactors;
        SlaveReactorManager::Ptr m_slaveReactorManager;
        components::ThreadPool::Ptr m_packetProcesser;

        HostsInfoManager::Ptr m_hostsInfoManager;
        HostsConnector::Ptr m_hostsConnector;
        HostsHeartbeatService::Ptr m_hostsHeartbeatService;
    };

} // server

#endif //TCPNETWORK_SERVICECONFIG_H
