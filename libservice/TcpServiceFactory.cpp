//
// Created by root on 10/11/23.
//

#include "TcpServiceFactory.h"

#include "config/ServiceConfig.h"

namespace service
{

    TcpServiceFactory::TcpServiceFactory(tools::NodeConfig::Ptr nodeConfig) :
            m_nodeConfig(std::move(nodeConfig))
    {
    }

    TcpService::Ptr TcpServiceFactory::createTcpService()
    {
        components::SelectListenner::Ptr listenner;
        Acceptor::Ptr acceptor;
        if(false == m_nodeConfig->startAsClient())
        {
            // 作为服务端需要
            listenner = std::make_shared<components::SelectListenner>();
            acceptor = std::make_shared<Acceptor>();
        }
        // 服务端客户端都需要
        SlaveReactorManager::Ptr slaveReactorManager = std::make_shared<SlaveReactorManager>();
        components::ThreadPool::Ptr packetProcessor = std::make_shared<components::ThreadPool>();

        // 客户端需要
        HostsInfoManager::Ptr hostsInfoManager = std::make_shared<HostsInfoManager>();
        HostsConnector::Ptr hostsConnector = std::make_shared<HostsConnector>();
        HostsHeartbeatService::Ptr hostsHeartbeatService = std::make_shared<HostsHeartbeatService>();

        // 创建TcpServiceConfig
        ServiceConfig::Ptr serviceConfig = std::make_shared<ServiceConfig>(m_nodeConfig, listenner, acceptor, slaveReactorManager, packetProcessor,
                                                                           hostsInfoManager, hostsConnector, hostsHeartbeatService);
        return std::make_shared<TcpService>(serviceConfig);
    }

} // service