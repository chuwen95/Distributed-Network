//
// Created by ChuWen on 9/20/23.
//

#include "ServiceConfig.h"
#include "csm-utilities/StringTool.h"

using namespace csm::service;

ServiceConfig::ServiceConfig(tool::NodeConfig::Ptr nodeConfig, utilities::SelectListenner::Ptr listenner, Acceptor::Ptr acceptor,
                             TcpSessionManager::Ptr tcpSessionManager, ClientAliveChecker::Ptr clientAliveChecker, std::vector<SlaveReactor::Ptr> slaveReactors,
                             SessionDispatcher::Ptr sessionDispatcher, SessionDestroyer::Ptr sessionDestroyer, SessionDataProcessor::Ptr sessionDataProcessor,
                             HostsInfoManager::Ptr hostsInfoManager, HostsConnector::Ptr hostsConnector, HostsHeartbeatService::Ptr hostsHeartbeatService) :
        m_nodeConfig(std::move(nodeConfig)), m_listenner(std::move(listenner)), m_acceptor(std::move(acceptor)),
        m_tcpSessionManager(std::move(tcpSessionManager)), m_clientAliveChecker(std::move(clientAliveChecker)),
        m_slaveReactors(std::move(slaveReactors)), m_sessionDispatcher(std::move(sessionDispatcher)),
        m_sessionDestroyer(std::move(sessionDestroyer)), m_sessionDataProcessor(std::move(sessionDataProcessor)),
        m_hostsInfoManager(std::move(hostsInfoManager)), m_hostsConnector(std::move(hostsConnector)),
        m_hostsHeartbeatService(std::move(hostsHeartbeatService))
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

TcpSessionManager::Ptr ServiceConfig::tcpSessionManager()
{
    return m_tcpSessionManager;
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