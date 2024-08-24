//
// Created by ChuWen on 10/11/23.
//

#include "TcpServiceFactory.h"

#include "config/ServiceConfig.h"
#include "service/TcpSessionManager.h"
#include "service/ClientAliveChecker.h"
#include "service/SessionDispatcher.h"
#include "service/SessionDestroyer.h"
#include "service/SessionDataProcessor.h"

using namespace csm::service;

TcpServiceFactory::TcpServiceFactory(tool::NodeConfig::Ptr nodeConfig) :
        m_nodeConfig(std::move(nodeConfig))
{
}

TcpService::Ptr TcpServiceFactory::createTcpService()
{
    utilities::SelectListenner::Ptr listenner;
    Acceptor::Ptr acceptor;
    if (false == m_nodeConfig->startAsClient())
    {
        // 作为服务端需要
        listenner = std::make_shared<utilities::SelectListenner>();
        acceptor = std::make_shared<Acceptor>();
    }
    // 创建TcpSession管理器
    TcpSessionManager::Ptr tcpSessionManager = std::make_shared<TcpSessionManager>();
    // 创建心跳检查器
    ClientAliveChecker::Ptr clientAliveChecker = std::make_shared<ClientAliveChecker>();

    // 服务端客户端都需要
    std::vector<SlaveReactor::Ptr> slaveReactors;
    for (int index = 0; index < m_nodeConfig->slaveReactorNum(); ++index)
    {
        slaveReactors.emplace_back(std::make_shared<SlaveReactor>(index, m_nodeConfig->id(), tcpSessionManager));
    }
    // TcpSession派发器
    SessionDispatcher::Ptr sessionDispatcher =
            std::make_shared<SessionDispatcher>(m_nodeConfig->redispatchInterval(), m_nodeConfig->id(), m_nodeConfig->slaveReactorNum());
    // TcpSession销毁器
    SessionDestroyer::Ptr sessionDestroyer = std::make_shared<SessionDestroyer>();
    // 包处理器
    SessionDataProcessor::Ptr sessionDataProcessor = std::make_shared<SessionDataProcessor>(tcpSessionManager, m_nodeConfig->sessionDataWorkerNum());

    // 客户端需要
    HostsInfoManager::Ptr hostsInfoManager = std::make_shared<HostsInfoManager>();
    HostsConnector::Ptr hostsConnector = std::make_shared<HostsConnector>();
    HostsHeartbeatService::Ptr hostsHeartbeatService = std::make_shared<HostsHeartbeatService>();

    // 创建TcpServiceConfig
    ServiceConfig::Ptr serviceConfig = std::make_shared<ServiceConfig>(m_nodeConfig, listenner, acceptor,
                                                                       tcpSessionManager, clientAliveChecker, slaveReactors,
                                                                       sessionDispatcher, sessionDestroyer, sessionDataProcessor,
                                                                       hostsInfoManager, hostsConnector, hostsHeartbeatService);
    return std::make_shared<TcpService>(serviceConfig);
}