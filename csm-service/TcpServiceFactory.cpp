//
// Created by ChuWen on 10/11/23.
//

#include "TcpServiceFactory.h"

#include "config/ServiceConfig.h"
#include "service/TcpSessionManager.h"

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

    // 服务端客户端都需要
    std::vector<SlaveReactor::Ptr> slaveReactors;
    for (int index = 0; index < m_nodeConfig->slaveReactorNum(); ++index)
    {
        slaveReactors.emplace_back(std::make_shared<SlaveReactor>(index, m_nodeConfig->id(), tcpSessionManager));
    }
    SlaveReactorManager::Ptr slaveReactorManager =
            std::make_shared<SlaveReactorManager>(m_nodeConfig->redispatchInterval(), m_nodeConfig->id(), slaveReactors);
    utilities::ThreadPool::Ptr packetProcessor = std::make_shared<utilities::ThreadPool>();

    // 客户端需要
    HostsInfoManager::Ptr hostsInfoManager = std::make_shared<HostsInfoManager>();
    HostsConnector::Ptr hostsConnector = std::make_shared<HostsConnector>();
    HostsHeartbeatService::Ptr hostsHeartbeatService = std::make_shared<HostsHeartbeatService>();

    // 创建TcpServiceConfig
    ServiceConfig::Ptr serviceConfig = std::make_shared<ServiceConfig>(m_nodeConfig, listenner, acceptor,
                                                                       tcpSessionManager,
                                                                       slaveReactors, slaveReactorManager, packetProcessor,
                                                                       hostsInfoManager, hostsConnector, hostsHeartbeatService);
    return std::make_shared<TcpService>(serviceConfig);
}