//
// Created by ChuWen on 10/11/23.
//

#include "P2PServiceFactory.h"

#include "config/ServiceConfig.h"
#include "service/P2PSessionManager.h"
#include "service/ClientAliveChecker.h"
#include "service/SessionDispatcher.h"
#include "service/SessionDestroyer.h"
#include "service/SessionDataProcessor.h"

using namespace csm::service;

P2PServiceFactory::P2PServiceFactory(tool::NodeConfig::Ptr nodeConfig, ServiceStartType serviceStartType) :
    m_nodeConfig(std::move(nodeConfig)), m_serviceStartType(serviceStartType)
{
}

P2PService::Ptr P2PServiceFactory::create()
{
    utilities::SelectListenner::Ptr listenner = std::make_shared<utilities::SelectListenner>();
    Acceptor::Ptr acceptor = std::make_shared<Acceptor>();
    // 创建P2PSession管理器
    P2PSessionManager::Ptr p2pSessionManager = std::make_shared<P2PSessionManager>();
    // 创建心跳检查器
    ClientAliveChecker::Ptr clientAliveChecker = std::make_shared<ClientAliveChecker>();
    // 创建子Reactor
    std::vector<SlaveReactor::Ptr> slaveReactors;
    for (int index = 0; index < m_nodeConfig->slaveReactorNum(); ++index)
    {
        slaveReactors.emplace_back(std::make_shared<SlaveReactor>(index, m_nodeConfig->id(), p2pSessionManager));
    }
    // P2PSession派发器
    SessionDispatcher::Ptr sessionDispatcher =
            std::make_shared<SessionDispatcher>(m_nodeConfig->redispatchInterval(), m_nodeConfig->id(), m_nodeConfig->slaveReactorNum());
    // P2PSession销毁器
    SessionDestroyer::Ptr sessionDestroyer = std::make_shared<SessionDestroyer>();
    // 包处理器
    SessionDataProcessor::Ptr sessionDataProcessor =
        std::make_shared<SessionDataProcessor>(p2pSessionManager, clientAliveChecker ,m_nodeConfig->sessionDataWorkerNum());

    ServiceConfig::Ptr serviceConfig{ nullptr };
    if(ServiceStartType::Node == m_serviceStartType)
    {
        // 作为节点需要
        HostsInfoManager::Ptr hostsInfoManager = std::make_shared<HostsInfoManager>(m_nodeConfig->nodesFile());
        HostsConnector::Ptr hostsConnector = std::make_shared<HostsConnector>(hostsInfoManager);
        HostsHeartbeatService::Ptr hostsHeartbeatService = std::make_shared<HostsHeartbeatService>(m_nodeConfig->id(), hostsInfoManager);

        // 创建TcpServiceConfig
         serviceConfig = std::make_shared<ServiceConfig>(m_nodeConfig, listenner, acceptor,
             p2pSessionManager, clientAliveChecker, slaveReactors,
             sessionDispatcher, sessionDestroyer, sessionDataProcessor,
             m_serviceStartType, hostsInfoManager, hostsConnector, hostsHeartbeatService);
    }
    else if(ServiceStartType::Server == m_serviceStartType)
    {
        // 创建TcpServiceConfig
        serviceConfig = std::make_shared<ServiceConfig>(m_nodeConfig, listenner, acceptor,
            p2pSessionManager, clientAliveChecker, slaveReactors,
            sessionDispatcher, sessionDestroyer, sessionDataProcessor,
            m_serviceStartType);
    }
    else
    {
        return nullptr;
    }

    return std::make_shared<P2PService>(serviceConfig);
}