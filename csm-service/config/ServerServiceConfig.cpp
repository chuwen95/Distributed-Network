//
// Created by ChuWen on 9/20/23.
//

#include "ServerServiceConfig.h"

using namespace csm::service;

ServerServiceConfig::ServerServiceConfig(tool::NodeConfig* nodeConfig, std::unique_ptr<utilities::SelectListenner> listenner, std::unique_ptr<Acceptor> acceptor,
                          std::unique_ptr<SessionAliveChecker> sessionAliveChecker,
                          std::unique_ptr<SlaveReactorPool> slaveReactorPool, std::unique_ptr<SessionDispatcher> sessionDispatcher,
                          std::unique_ptr<SessionDestroyer> sessionDestroyer, std::unique_ptr<SessionDataDecoder> sessionDataDecoder,
                          std::unique_ptr<SessionServiceDataProcessor> sessionServiceDataProcesser,
                          std::unique_ptr<SessionModuleDataProcessor> sessionModuleDataProcessor)
    : m_nodeConfig(nodeConfig), m_listenner(std::move(listenner)), m_acceptor(std::move(acceptor)),
      m_sessionAliveChecker(std::move(sessionAliveChecker)),
      m_slaveReactorPool(std::move(slaveReactorPool)), m_sessionDispatcher(std::move(sessionDispatcher)),
      m_sessionDestroyer(std::move(sessionDestroyer)), m_sessionDataDecoder(std::move(sessionDataDecoder)),
      m_sessionServiceDataProcessor(std::move(sessionServiceDataProcesser)),
      m_sessionModuleDataProcessor(std::move(sessionModuleDataProcessor))
{
}

csm::tool::NodeConfig* ServerServiceConfig::nodeConfig()
{
    return m_nodeConfig;
}

csm::utilities::SelectListenner* ServerServiceConfig::listenner()
{
    return m_listenner.get();
}
Acceptor* ServerServiceConfig::acceptor()
{
    return m_acceptor.get();
}

SessionAliveChecker* ServerServiceConfig::sessionAliveChecker()
{
    return m_sessionAliveChecker.get();
}

SlaveReactorPool* ServerServiceConfig::slaveReactorPool()
{
    return m_slaveReactorPool.get();
}
SessionDispatcher* ServerServiceConfig::sessionDispatcher()
{
    return m_sessionDispatcher.get();
}
SessionDestroyer* ServerServiceConfig::sessionDestroyer()
{
    return m_sessionDestroyer.get();
}

SessionDataDecoder* ServerServiceConfig::sessionDataDecoder()
{
    return m_sessionDataDecoder.get();
}
SessionServiceDataProcessor* ServerServiceConfig::sessionServiceDataProcessor()
{
    return m_sessionServiceDataProcessor.get();
}
SessionModuleDataProcessor* ServerServiceConfig::sessionModuleDataProcessor()
{
    return m_sessionModuleDataProcessor.get();
}