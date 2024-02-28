//
// Created by root on 9/20/23.
//

#include "ServiceConfig.h"
#include "libcomponents/StringTool.h"

namespace service
{

    ServiceConfig::ServiceConfig(tools::NodeConfig::Ptr nodeConfig, components::SelectListenner::Ptr listenner, Acceptor::Ptr acceptor,
                                 std::vector<SlaveReactor::Ptr> slaveReactors, SlaveReactorManager::Ptr slaveReactorManager, components::ThreadPool::Ptr packetProcessor,
                                 HostsInfoManager::Ptr hostsInfoManager, HostsConnector::Ptr hostsConnector, HostsHeartbeatService::Ptr hostsHeartbeatService) :
                                 m_nodeConfig(std::move(nodeConfig)), m_listenner(std::move(listenner)), m_acceptor(std::move(acceptor)),
                                 m_slaveReactors(std::move(slaveReactors)), m_slaveReactorManager(std::move(slaveReactorManager)), m_packetProcesser(std::move(packetProcessor)),
                                 m_hostsInfoManager(std::move(hostsInfoManager)), m_hostsConnector(std::move(hostsConnector)),
                                 m_hostsHeartbeatService(std::move(hostsHeartbeatService))
    {}

    tools::NodeConfig::Ptr ServiceConfig::nodeConfig()
    {
        return m_nodeConfig;
    }

    components::SelectListenner::Ptr ServiceConfig::listenner()
    {
        return m_listenner;
    }

    Acceptor::Ptr ServiceConfig::acceptor()
    {
        return m_acceptor;
    }

    std::vector<SlaveReactor::Ptr>& ServiceConfig::slaveReactors()
    {
        return m_slaveReactors;
    }

    SlaveReactorManager::Ptr ServiceConfig::slaveReactorManager()
    {
        return m_slaveReactorManager;
    }

    components::ThreadPool::Ptr ServiceConfig::packetProcessor()
    {
        return m_packetProcesser;
    }

    HostsInfoManager::Ptr ServiceConfig::hostsInfoManager()
    {
        return m_hostsInfoManager;
    }

    HostsConnector::Ptr ServiceConfig::hostsConnector()
    {
        return m_hostsConnector;
    }

    HostsHeartbeatService::Ptr ServiceConfig::hostsHeartbeatService()
    {
        return m_hostsHeartbeatService;
    }

}