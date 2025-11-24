//
// Created by ChuWen on 9/20/23.
//

#ifndef TCPNETWORK_P2PSERVICECONFIG_H
#define TCPNETWORK_P2PSERVICECONFIG_H

#include "csm-tool/NodeConfig.h"
#include "csm-utilities/SelectListenner.h"
#include "csm-service/service/Acceptor.h"
#include "csm-service/service/SessionAliveChecker.h"
#include "csm-service/service/SessionDispatcher.h"
#include "csm-service/service/SlaveReactorPool.h"
#include "csm-service/service/SessionDestroyer.h"
#include "csm-service/service/SessionDataDecoder.h"
#include "csm-service/service/SessionServiceDataProcessor.h"
#include "csm-service/service/SessionModuleDataProcessor.h"

#include "csm-service/host/HostsConnector.h"
#include "csm-service/host/HostsInfoManager.h"
#include "csm-service/host/HostsHeartbeatService.h"

#include "csm-service/routing_algorithm/distance_vector/DistanceVector.h"

namespace csm
{

    namespace service
    {

        using ModulePacketHandler = std::function<int(const NodeId& nodeId, const std::vector<char>&)>;

        class P2PServiceConfig
        {
        public:
            explicit P2PServiceConfig(tool::NodeConfig* nodeConfig, std::unique_ptr<utilities::SelectListenner> listenner,
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
                                      std::unique_ptr<DistanceVector> distanceVector);
            ~P2PServiceConfig() = default;

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

            HostsInfoManager* hostsInfoManager();
            HostsConnector* hostsConnector();
            HostsHeartbeatService* hostsHeartbeatService();

            DistanceVector* distanceVector();

            void registerModulePacketHandler(protocol::ModuleID moduleId, ModulePacketHandler modulePacketHandler);
            int modulePacketHandler(protocol::ModuleID moduleId, ModulePacketHandler& modulePacketHandler);

        private:
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

            std::unique_ptr<HostsInfoManager> m_hostsInfoManager;
            std::unique_ptr<HostsConnector> m_hostsConnector;
            std::unique_ptr<HostsHeartbeatService> m_hostsHeartbeatService;

            std::unique_ptr<DistanceVector> m_distanceVector;

            std::mutex x_modulePacketHandler;
            std::map<protocol::ModuleID, ModulePacketHandler> m_modulePacketHandler;
        };

    } // service

}

#endif //TCPNETWORK_P2PSERVICECONFIG_H