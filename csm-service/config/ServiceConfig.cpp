//
// Created by root on 9/20/23.
//

#include "ServiceConfig.h"
#include "csm-utilities/StringTool.h"

using namespace csm::service;

ServiceConfig::ServiceConfig(tool::NodeConfig::Ptr nodeConfig, utilities::SelectListenner::Ptr listenner,
                             Acceptor::Ptr acceptor,
                             std::vector<SlaveReactor::Ptr> slaveReactors, SlaveReactorManager::Ptr slaveReactorManager,
                             utilities::ThreadPool::Ptr packetProcessor,
                             HostsInfoManager::Ptr hostsInfoManager, HostsConnector::Ptr hostsConnector,
                             HostsHeartbeatService::Ptr hostsHeartbeatService) :
        m_nodeConfig(std::move(nodeConfig)), m_listenner(std::move(listenner)), m_acceptor(std::move(acceptor)),
        m_slaveReactors(std::move(slaveReactors)), m_slaveReactorManager(std::move(slaveReactorManager)),
        m_packetProcesser(std::move(packetProcessor)),
        m_hostsInfoManager(std::move(hostsInfoManager)), m_hostsConnector(std::move(hostsConnector)),
        m_hostsHeartbeatService(std::move(hostsHeartbeatService))
{}

tool::NodeConfig::Ptr ServiceConfig::nodeConfig()
{
    return m_nodeConfig;
}

csm::utilities::SelectListenner::Ptr ServiceConfig::listenner()
{
    return m_listenner;
}

Acceptor::Ptr ServiceConfig::acceptor()
{
    return m_acceptor;
}

std::vector<SlaveReactor::Ptr> &ServiceConfig::slaveReactors()
{
    return m_slaveReactors;
}

SlaveReactorManager::Ptr ServiceConfig::slaveReactorManager()
{
    return m_slaveReactorManager;
}

csm::utilities::ThreadPool::Ptr ServiceConfig::packetProcessor()
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