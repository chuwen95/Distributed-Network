//
// Created by ChuWen on 9/20/23.
//

#include "P2PServiceConfig.h"

using namespace csm::service;

P2PServiceConfig::P2PServiceConfig(tool::NodeConfig* nodeConfig, std::unique_ptr<utilities::SelectListenner> listenner,
                                      std::unique_ptr<Acceptor> acceptor,
                                      std::unique_ptr<SessionAliveChecker> sessionAliveChecker,
                                      std::unique_ptr<SlaveReactorPool> slaveReactorPool,
                                      std::unique_ptr<SessionDispatcher> sessionDispatcher,
                                      std::unique_ptr<SessionDestroyer> sessionDestroyer,
                                      std::unique_ptr<SessionDataDecoder> sessionDataDecoder,
                                      std::unique_ptr<SessionServiceDataProcessor> sessionServiceDataProcesser,
                                      std::unique_ptr<SessionModuleDataProcessor> sessionModuleDataProcessor,
                                      std::unique_ptr<HostsInfoManager> hostsInfoManager,
                                      std::unique_ptr<HostsConnector> hostsConnector,
                                      std::unique_ptr<HostsHeartbeatService> hostHeartbeatService,
                                      std::unique_ptr<DistanceVector> distanceVector)
    : m_nodeConfig(nodeConfig), m_listenner(std::move(listenner)),
      m_acceptor(std::move(acceptor)), m_sessionAliveChecker(std::move(sessionAliveChecker)),
      m_slaveReactorPool(std::move(slaveReactorPool)),
      m_sessionDispatcher(std::move(sessionDispatcher)), m_sessionDestroyer(std::move(sessionDestroyer)),
      m_sessionDataDecoder(std::move(sessionDataDecoder)),
      m_sessionServiceDataProcessor(std::move(sessionServiceDataProcesser)),
      m_sessionModuleDataProcessor(std::move(sessionModuleDataProcessor)),
      m_hostsInfoManager(std::move(hostsInfoManager)), m_hostsConnector(std::move(hostsConnector)),
      m_hostsHeartbeatService(std::move(hostHeartbeatService)),
      m_distanceVector(std::move(distanceVector))
{
}

csm::tool::NodeConfig* P2PServiceConfig::nodeConfig()
{
    return m_nodeConfig;
}

csm::utilities::SelectListenner* P2PServiceConfig::listenner()
{
    return m_listenner.get();
}

Acceptor* P2PServiceConfig::acceptor()
{
    return m_acceptor.get();
}

SessionAliveChecker* P2PServiceConfig::sessionAliveChecker()
{
    return m_sessionAliveChecker.get();
}

SlaveReactorPool* P2PServiceConfig::slaveReactorPool()
{
    return m_slaveReactorPool.get();
}

SessionDispatcher* P2PServiceConfig::sessionDispatcher()
{
    return m_sessionDispatcher.get();
}

SessionDestroyer* P2PServiceConfig::sessionDestroyer()
{
    return m_sessionDestroyer.get();
}

SessionDataDecoder* P2PServiceConfig::sessionDataDecoder()
{
    return m_sessionDataDecoder.get();
}

SessionServiceDataProcessor* P2PServiceConfig::sessionServiceDataProcessor()
{
    return m_sessionServiceDataProcessor.get();
}

SessionModuleDataProcessor* P2PServiceConfig::sessionModuleDataProcessor()
{
    return m_sessionModuleDataProcessor.get();
}

HostsConnector* P2PServiceConfig::hostsConnector()
{
    return m_hostsConnector.get();
}

HostsInfoManager* P2PServiceConfig::hostsInfoManager()
{
    return m_hostsInfoManager.get();
}

HostsHeartbeatService* P2PServiceConfig::hostsHeartbeatService()
{
    return m_hostsHeartbeatService.get();
}

DistanceVector* P2PServiceConfig::distanceVector()
{
    return m_distanceVector.get();
}

void P2PServiceConfig::registerModulePacketHandler(const protocol::ModuleID moduleId, ModulePacketHandler modulePacketHandler)
{
    std::unique_lock<std::mutex> ulock(x_modulePacketHandler);
    m_modulePacketHandler[moduleId] = std::move(modulePacketHandler);
}

int P2PServiceConfig::modulePacketHandler(const protocol::ModuleID moduleId, ModulePacketHandler& modulePacketHandler)
{
    std::unique_lock<std::mutex> ulock(x_modulePacketHandler);

    auto iter = m_modulePacketHandler.find(moduleId);
    if (m_modulePacketHandler.end() == iter)
    {
        LOG->write(csm::utilities::LogType::Log_Error, FILE_INFO, "not found module package handler, moduleId: ", moduleId);
        return -1;
    }

    modulePacketHandler = iter->second;
    return 0;
}