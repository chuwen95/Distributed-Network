//
// Created by ChuWen on 10/11/23.
//

#include "P2PServiceFactory.h"

#include "config/ServiceConfig.h"
#include "service/P2PSessionManager.h"
#include "service/SessionAliveChecker.h"
#include "service/SessionDispatcher.h"
#include "service/SessionDestroyer.h"
#include "service/SessionModuleDataProcessor.h"
#include "protocol/payload/PayloadFactory.h"
#include "routing_algorithm/distance_vector/DistanceVector.h"

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
    SessionAliveChecker::Ptr sessionAliveChecker = std::make_shared<SessionAliveChecker>();
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
    // 包解码器
    SessionDataDecoder::Ptr sessionDataDecoder =
        createSessionDataDecoder(p2pSessionManager, m_nodeConfig->sessionDataDecoderWorkerNum());
    // 网络模组包处理器
    SessionServiceDataProcessor::Ptr sessionSericeDataProcessor = createServiceDataProcessor();
    // 其他模组包处理器
    SessionModuleDataProcessor::Ptr sessionModuleDataProcessor =
        createModuleDataProcessor(p2pSessionManager, m_nodeConfig->moduleDataProcessWorkerNum());

    ServiceConfig::Ptr serviceConfig{ nullptr };
    if(ServiceStartType::Node == m_serviceStartType)
    {
        // 作为节点需要
        HostsInfoManager::Ptr hostsInfoManager = std::make_shared<HostsInfoManager>(m_nodeConfig->nodesFile());
        HostsConnector::Ptr hostsConnector = std::make_shared<HostsConnector>(hostsInfoManager);
        HostsHeartbeatService::Ptr hostsHeartbeatService = std::make_shared<HostsHeartbeatService>(m_nodeConfig->id(), hostsInfoManager);

        // 路由选择算法
        DistanceVector::Ptr distanceVector = createDistanceVector();

        // 创建TcpServiceConfig
         serviceConfig = std::make_shared<ServiceConfig>(m_nodeConfig, listenner, acceptor,
             p2pSessionManager, sessionAliveChecker, slaveReactors,
             sessionDispatcher, sessionDestroyer, sessionDataDecoder, sessionSericeDataProcessor, sessionModuleDataProcessor,
             m_serviceStartType, hostsInfoManager, hostsConnector, hostsHeartbeatService, distanceVector);
    }
    else if(ServiceStartType::RpcServer == m_serviceStartType)
    {
        // 创建TcpServiceConfig
        serviceConfig = std::make_shared<ServiceConfig>(m_nodeConfig, listenner, acceptor,
            p2pSessionManager, sessionAliveChecker, slaveReactors, sessionDispatcher,
            sessionDestroyer, sessionDataDecoder, sessionSericeDataProcessor, sessionModuleDataProcessor, m_serviceStartType);
    }
    else
    {
        return nullptr;
    }

    return std::make_shared<P2PService>(serviceConfig);
}

SessionDataDecoder::Ptr P2PServiceFactory::createSessionDataDecoder(
    P2PSessionManager::Ptr p2pSessionManager, const std::size_t workerNum)
{
    PayloadFactory::Ptr payloadFactory = std::make_shared<PayloadFactory>();
    utilities::ThreadPool::Ptr sessionDataDecoder =
        std::make_shared<utilities::ThreadPool>(m_nodeConfig->sessionDataDecoderWorkerNum(), "sess_dt_deco");

    return std::make_shared<SessionDataDecoder>(std::move(p2pSessionManager), payloadFactory, sessionDataDecoder);
}

SessionServiceDataProcessor::Ptr P2PServiceFactory::createServiceDataProcessor()
{
    utilities::Thread::Ptr thread = std::make_shared<utilities::Thread>();
    return std::make_shared<SessionServiceDataProcessor>(thread);
}

SessionModuleDataProcessor::Ptr P2PServiceFactory::createModuleDataProcessor(
    P2PSessionManager::Ptr p2pSessionManager, std::size_t workerNum)
{
    utilities::ThreadPool::Ptr normalPacketProcessor = std::make_shared<utilities::ThreadPool>(workerNum, "sess_dt_proc");
    return std::make_shared<SessionModuleDataProcessor>(std::move(p2pSessionManager), normalPacketProcessor);
}

DistanceVector::Ptr P2PServiceFactory::createDistanceVector()
{
    utilities::Thread::Ptr thread = std::make_shared<utilities::Thread>();
    return std::make_shared<DistanceVector>(thread);
}
