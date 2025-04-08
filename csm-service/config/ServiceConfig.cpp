//
// Created by ChuWen on 9/20/23.
//

#include "ServiceConfig.h"
#include "csm-utilities/StringTool.h"

using namespace csm::service;

ServiceConfig::ServiceConfig(tool::NodeConfig::Ptr nodeConfig, utilities::SelectListenner::Ptr listenner, Acceptor::Ptr acceptor,
                             P2PSessionManager::Ptr p2pSessionManager, ClientAliveChecker::Ptr clientAliveChecker, std::vector<SlaveReactor::Ptr> slaveReactors,
                             SessionDispatcher::Ptr sessionDispatcher, SessionDestroyer::Ptr sessionDestroyer, SessionDataProcessor::Ptr sessionDataProcessor,
                             const ServiceStartType serverStartType, HostsInfoManager::Ptr hostsInfoManager, HostsConnector::Ptr hostsConnector,
                             HostsHeartbeatService::Ptr hostsHeartbeatService) :
        m_nodeConfig(std::move(nodeConfig)), m_listenner(std::move(listenner)), m_acceptor(std::move(acceptor)),
        m_p2pSessionManager(std::move(p2pSessionManager)), m_clientAliveChecker(std::move(clientAliveChecker)),
        m_slaveReactors(std::move(slaveReactors)), m_sessionDispatcher(std::move(sessionDispatcher)),
        m_sessionDestroyer(std::move(sessionDestroyer)), m_sessionDataProcessor(std::move(sessionDataProcessor)),
        m_serviceStartType(serverStartType), m_hostsInfoManager(std::move(hostsInfoManager)),
        m_hostsConnector(std::move(hostsConnector)), m_hostsHeartbeatService(std::move(hostsHeartbeatService))
{}

csm::tool::NodeConfig::Ptr ServiceConfig::nodeConfig()
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

P2PSessionManager::Ptr ServiceConfig::p2pSessionManager()
{
    return m_p2pSessionManager;
}

ClientAliveChecker::Ptr ServiceConfig::clientAliveChecker()
{
    return m_clientAliveChecker;
}

std::vector<SlaveReactor::Ptr>& ServiceConfig::slaveReactors()
{
    return m_slaveReactors;
}

SessionDispatcher::Ptr ServiceConfig::sessionDispatcher()
{
    return m_sessionDispatcher;
}

SessionDestroyer::Ptr ServiceConfig::sessionDestroyer()
{
    return m_sessionDestroyer;
}

SessionDataProcessor::Ptr ServiceConfig::sessionDataProcessor()
{
    return m_sessionDataProcessor;
}

ServiceStartType ServiceConfig::serviceStartType() const
{
    return m_serviceStartType;
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

void ServiceConfig::registerModulePacketHandler(const std::int32_t moduleId, ModulePacketHandler modulePacketHandler)
{
    std::unique_lock<std::mutex> ulock(x_modulePacketHandler);

    m_modulePacketHandler[moduleId] = std::move(modulePacketHandler);
}

std::function<int(std::shared_ptr<std::vector<char>>)> ServiceConfig::getModulePacketHandler(const std::int32_t moduleId)
{
    std::unique_lock<std::mutex> ulock(x_modulePacketHandler);

    auto iter = m_modulePacketHandler.find(moduleId);
    if (m_modulePacketHandler.end() == iter)
    {
        LOG->write(csm::utilities::LogType::Log_Error, FILE_INFO, "not found module package handler, moduleId: ", moduleId);
        return nullptr;
    }

    return iter->second;
}
