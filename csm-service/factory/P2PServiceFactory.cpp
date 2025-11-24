//
// Created by ChuWen on 10/11/23.
//

#include "csm-service/factory/P2PServiceFactory.h"

#include "csm-service/config/P2PServiceConfig.h"
#include "csm-service/protocol/payload/PayloadFactory.h"
#include "csm-service/routing_algorithm/distance_vector/DistanceVector.h"
#include "csm-service/service/P2PSessionFactory.h"
#include "csm-service/service/SessionAliveChecker.h"
#include "csm-service/service/SessionDestroyer.h"
#include "csm-service/service/SessionDispatcher.h"
#include "csm-service/service/SessionModuleDataProcessor.h"
#include "csm-service/service/P2PService.h"

using namespace csm::service;

P2PServiceFactory::P2PServiceFactory(tool::NodeConfig* nodeConfig) : m_nodeConfig(nodeConfig)
{
}

std::unique_ptr<P2PService> P2PServiceFactory::create()
{
    // 创建Listenner
    std::unique_ptr<utilities::SelectListenner> listenner = std::make_unique<utilities::SelectListenner>();
    // 创建P2PSession工厂
    std::shared_ptr<P2PSessionFactory> p2pSessionFactory = std::make_shared<P2PSessionFactory>();
    // 创建Acceptor
    std::unique_ptr<Acceptor> acceptor = std::make_unique<Acceptor>(p2pSessionFactory);
    // 创建心跳检查器
    std::unique_ptr<SessionAliveChecker> sessionAliveChecker = std::make_unique<SessionAliveChecker>();
    // 创建SlaveReactor池
    std::unique_ptr<SlaveReactorPool> slaveReactorPool = std::make_unique<SlaveReactorPool>(m_nodeConfig->slaveReactorNum());
    // P2PSession派发器
    std::unique_ptr<SessionDispatcher> sessionDispatcher = std::make_unique<SessionDispatcher>(m_nodeConfig->slaveReactorNum());
    // P2PSession销毁器
    std::unique_ptr<SessionDestroyer> sessionDestroyer = std::make_unique<SessionDestroyer>();
    // 包解码器
    std::unique_ptr<SessionDataDecoder> sessionDataDecoder =
        createSessionDataDecoder(m_nodeConfig->sessionDataDecoderWorkerNum());
    // 网络模组包处理器
    std::unique_ptr<SessionServiceDataProcessor> sessionSericeDataProcessor = createServiceDataProcessor();
    // 其他模组包处理器
    std::unique_ptr<SessionModuleDataProcessor> sessionModuleDataProcessor =
        createModuleDataProcessor(m_nodeConfig->moduleDataProcessWorkerNum());

    // 作为节点需要
    std::unique_ptr<HostsInfoManager> hostsInfoManager = std::make_unique<HostsInfoManager>(m_nodeConfig->nodesFile());
    std::unique_ptr<HostsConnector> hostsConnector = std::make_unique<HostsConnector>(hostsInfoManager.get(), p2pSessionFactory);
    std::unique_ptr<HostsHeartbeatService> hostsHeartbeatService =
        std::make_unique<HostsHeartbeatService>(m_nodeConfig->nodeId(), hostsInfoManager.get());

    // 路由选择算法
    std::unique_ptr<DistanceVector> distanceVector = createDistanceVector();

    // 创建TcpServiceConfig
    std::unique_ptr<P2PServiceConfig> serviceConfig = std::make_unique<P2PServiceConfig>(
        m_nodeConfig, std::move(listenner), std::move(acceptor), std::move(sessionAliveChecker), std::move(slaveReactorPool),
        std::move(sessionDispatcher),
        std::move(sessionDestroyer), std::move(sessionDataDecoder), std::move(sessionSericeDataProcessor),
        std::move(sessionModuleDataProcessor),
        std::move(hostsInfoManager), std::move(hostsConnector), std::move(hostsHeartbeatService), std::move(distanceVector));

    return std::make_unique<P2PService>(std::move(serviceConfig));
}

std::unique_ptr<SessionDataDecoder> P2PServiceFactory::createSessionDataDecoder(const std::size_t workerNum)
{
    PayloadFactory::Ptr payloadFactory = std::make_shared<PayloadFactory>();
    utilities::ThreadPool::Ptr sessionDataDecoder =
        std::make_shared<utilities::ThreadPool>(m_nodeConfig->sessionDataDecoderWorkerNum(), "sess_dt_deco");

    return std::make_unique<SessionDataDecoder>(payloadFactory, sessionDataDecoder);
}

std::unique_ptr<SessionServiceDataProcessor> P2PServiceFactory::createServiceDataProcessor()
{
    utilities::Thread::Ptr thread = std::make_shared<utilities::Thread>();
    return std::make_unique<SessionServiceDataProcessor>(thread);
}

std::unique_ptr<SessionModuleDataProcessor> P2PServiceFactory::createModuleDataProcessor(std::size_t workerNum)
{
    utilities::ThreadPool::Ptr normalPacketProcessor = std::make_shared<utilities::ThreadPool>(workerNum, "sess_dt_proc");
    return std::make_unique<SessionModuleDataProcessor>(normalPacketProcessor);
}

std::unique_ptr<DistanceVector> P2PServiceFactory::createDistanceVector()
{
    utilities::Thread::Ptr thread = std::make_shared<utilities::Thread>();
    return std::make_unique<DistanceVector>(thread);
}