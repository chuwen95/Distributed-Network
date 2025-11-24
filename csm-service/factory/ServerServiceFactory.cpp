//
// Created by ChuWen on 10/11/23.
//

#include "csm-service/factory/ServerServiceFactory.h"

#include "csm-service/config/ServerServiceConfig.h"
#include "csm-service/protocol/payload/PayloadFactory.h"
#include "csm-service/service/P2PSessionFactory.h"
#include "csm-service/service/ServerService.h"
#include "csm-service/service/SessionAliveChecker.h"
#include "csm-service/service/SessionDestroyer.h"
#include "csm-service/service/SessionDispatcher.h"
#include "csm-service/service/SessionModuleDataProcessor.h"

using namespace csm::service;

ServerServiceFactory::ServerServiceFactory(tool::NodeConfig* nodeConfig) : m_nodeConfig(nodeConfig)
{
}

std::unique_ptr<ServerService> ServerServiceFactory::create()
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

    // 创建TcpServiceConfig
    std::unique_ptr<ServerServiceConfig> serviceConfig = std::make_unique<ServerServiceConfig>(
        m_nodeConfig, std::move(listenner), std::move(acceptor), std::move(sessionAliveChecker), std::move(slaveReactorPool),
        std::move(sessionDispatcher),
        std::move(sessionDestroyer), std::move(sessionDataDecoder), std::move(sessionSericeDataProcessor),
        std::move(sessionModuleDataProcessor));

    return std::make_unique<ServerService>(std::move(serviceConfig));
}

std::unique_ptr<SessionDataDecoder> ServerServiceFactory::createSessionDataDecoder(const std::size_t workerNum)
{
    PayloadFactory::Ptr payloadFactory = std::make_shared<PayloadFactory>();
    utilities::ThreadPool::Ptr sessionDataDecoder =
        std::make_shared<utilities::ThreadPool>(m_nodeConfig->sessionDataDecoderWorkerNum(), "sess_dt_deco");

    return std::make_unique<SessionDataDecoder>(payloadFactory, sessionDataDecoder);
}

std::unique_ptr<SessionServiceDataProcessor> ServerServiceFactory::createServiceDataProcessor()
{
    utilities::Thread::Ptr thread = std::make_shared<utilities::Thread>();
    return std::make_unique<SessionServiceDataProcessor>(thread);
}

std::unique_ptr<SessionModuleDataProcessor> ServerServiceFactory::createModuleDataProcessor(std::size_t workerNum)
{
    utilities::ThreadPool::Ptr normalPacketProcessor = std::make_shared<utilities::ThreadPool>(workerNum, "sess_dt_proc");
    return std::make_unique<SessionModuleDataProcessor>(normalPacketProcessor);
}