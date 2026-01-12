//
// Created by ChuWen on 9/20/23.
//

#ifndef TCPNETWORK_SERVERSERVICECONFIG_H
#define TCPNETWORK_SERVERSERVICECONFIG_H

#include "csm-tool/NodeConfig.h"
#include "csm-utilities/SelectListenner.h"
#include "csm-service/service/Acceptor.h"
#include "csm-service/service/SlaveReactorPool.h"
#include "csm-service/service/SessionAliveChecker.h"
#include "csm-service/service/SessionDataDecoder.h"
#include "csm-service/service/SessionDestroyer.h"
#include "csm-service/service/SessionDispatcher.h"
#include "csm-service/service/SessionModuleDataProcessor.h"
#include "csm-service/service/SessionServiceDataProcessor.h"

namespace csm
{

    namespace service
    {

        class ServerServiceConfig
        {
          public:
            explicit ServerServiceConfig(tool::NodeConfig* nodeConfig, std::unique_ptr<utilities::SelectListenner> listenner, std::unique_ptr<Acceptor> acceptor,
                          std::unique_ptr<SessionAliveChecker> sessionAliveChecker,
                          std::unique_ptr<SlaveReactorPool> slaveReactorPool, std::unique_ptr<SessionDispatcher> sessionDispatcher,
                          std::unique_ptr<SessionDestroyer> sessionDestroyer, std::unique_ptr<SessionDataDecoder> sessionDataDecoder,
                          std::unique_ptr<SessionServiceDataProcessor> sessionServiceDataProcesser,
                          std::unique_ptr<SessionModuleDataProcessor> sessionModuleDataProcessor);
            ~ServerServiceConfig() = default;

          public:
            tool::NodeConfig* nodeConfig();

            utilities::SelectListenner* listenner();
            Acceptor* acceptor();

            SessionAliveChecker* sessionAliveChecker();

            SlaveReactorPool* slaveReactorPool();
            SessionDispatcher* sessionDispatcher();
            SessionDestroyer* sessionDestroyer();

            SessionDataDecoder* sessionDataDecoder();
            SessionServiceDataProcessor* sessionServiceDataProcessor();
            SessionModuleDataProcessor* sessionModuleDataProcessor();

          protected:
            tool::NodeConfig* m_nodeConfig;

            std::unique_ptr<utilities::SelectListenner> m_listenner;
            std::unique_ptr<Acceptor> m_acceptor;

            std::unique_ptr<SessionAliveChecker> m_sessionAliveChecker;

            std::unique_ptr<SlaveReactorPool> m_slaveReactorPool;
            std::unique_ptr<SessionDispatcher> m_sessionDispatcher;
            std::unique_ptr<SessionDestroyer> m_sessionDestroyer;

            std::unique_ptr<SessionDataDecoder> m_sessionDataDecoder;
            std::unique_ptr<SessionServiceDataProcessor> m_sessionServiceDataProcessor;
            std::unique_ptr<SessionModuleDataProcessor> m_sessionModuleDataProcessor;
        };

    } // server

}

#endif //TCPNETWORK_SERVERSERVICECONFIG_H
