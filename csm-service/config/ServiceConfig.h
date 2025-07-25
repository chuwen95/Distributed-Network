//
// Created by ChuWen on 9/20/23.
//

#ifndef TCPNETWORK_SERVICECONFIG_H
#define TCPNETWORK_SERVICECONFIG_H

#include "csm-common/Common.h"
#include "csm-tool/NodeConfig.h"
#include "inipp.h"

#include "csm-service/routing_algorithm/distance_vector/DistanceVector.h"
#include "csm-service/service/Acceptor.h"
#include "csm-service/service/SessionDataDecoder.h"
#include "csm-service/service/SessionDestroyer.h"
#include "csm-service/service/SessionDispatcher.h"
#include "csm-service/service/SessionModuleDataProcessor.h"
#include "csm-service/service/SessionServiceDataProcessor.h"
#include "csm-utilities/SelectListenner.h"

#include "csm-service/host/HostsConnector.h"
#include "csm-service/host/HostsHeartbeatService.h"
#include "csm-service/host/HostsInfoManager.h"

#include "csm-service/service/P2PSessionManager.h"
#include "csm-service/service/SessionAliveChecker.h"

#include "csm-framework/protocol/Protocol.h"

namespace csm
{

    namespace service
    {

        using ModulePacketHandler = std::function<int(const NodeId& nodeId, const std::vector<char>&)>;

        enum class ServiceStartType
        {
            Node,      // 作为节点启动，需要service模块全内容和host模块，节点相互连接
            RpcServer, // 作为服务器启动，为RPC模块所用，仅需要service模块部分功能，不包含握手协议部分
        };

        class ServiceConfig
        {
          public:
            using Ptr = std::shared_ptr<ServiceConfig>;

            ServiceConfig(tool::NodeConfig::Ptr nodeConfig, utilities::SelectListenner::Ptr listenner, Acceptor::Ptr acceptor,
                          P2PSessionManager::Ptr p2pSessionManager, SessionAliveChecker::Ptr sessionAliveChecker,
                          std::vector<SlaveReactor::Ptr> slaveReactors, SessionDispatcher::Ptr sessionDispatcher,
                          SessionDestroyer::Ptr sessionDestroyer, SessionDataDecoder::Ptr sessionDataDecoder,
                          SessionServiceDataProcessor::Ptr sessionServiceDataProcesser,
                          SessionModuleDataProcessor::Ptr sessionModuleDataProcessor,
                          ServiceStartType serverStartType = ServiceStartType::Node,
                          HostsInfoManager::Ptr hostsInfoManager = nullptr, HostsConnector::Ptr hostsConnector = nullptr,
                          HostsHeartbeatService::Ptr hostsHeartbeatService = nullptr,
                          DistanceVector::Ptr distanceVector = nullptr);
            ~ServiceConfig() = default;

          public:
            tool::NodeConfig::Ptr nodeConfig();

            utilities::SelectListenner::Ptr listenner();
            Acceptor::Ptr acceptor();

            P2PSessionManager::Ptr p2pSessionManager();
            SessionAliveChecker::Ptr sessionAliveChecker();

            std::vector<SlaveReactor::Ptr>& slaveReactors();
            SessionDispatcher::Ptr sessionDispatcher();
            SessionDestroyer::Ptr sessionDestroyer();

            SessionDataDecoder::Ptr sessionDataDecoder();
            SessionServiceDataProcessor::Ptr sessionServiceDataProcessor();
            SessionModuleDataProcessor::Ptr sessionModuleDataProcessor();

            ServiceStartType serviceStartType() const;

            HostsInfoManager::Ptr hostsInfoManager();
            HostsConnector::Ptr hostsConnector();
            HostsHeartbeatService::Ptr hostsHeartbeatService();

            void registerModulePacketHandler(protocol::ModuleID moduleId, ModulePacketHandler modulePacketHandler);
            int modulePacketHandler(protocol::ModuleID moduleId, ModulePacketHandler& modulePacketHandler);

            DistanceVector::Ptr distanceVector();

          private:
            tool::NodeConfig::Ptr m_nodeConfig;

            utilities::SelectListenner::Ptr m_listenner;
            Acceptor::Ptr m_acceptor;

            P2PSessionManager::Ptr m_p2pSessionManager;
            SessionAliveChecker::Ptr m_sessionAliveChecker;
            std::vector<SlaveReactor::Ptr> m_slaveReactors;

            SessionDispatcher::Ptr m_sessionDispatcher;
            SessionDestroyer::Ptr m_sessionDestroyer;

            SessionDataDecoder::Ptr m_sessionDataDecoder;
            SessionServiceDataProcessor::Ptr m_sessionServiceDataProcessor;
            SessionModuleDataProcessor::Ptr m_sessionModuleDataProcessor;

            ServiceStartType m_serviceStartType;

            HostsInfoManager::Ptr m_hostsInfoManager;
            HostsConnector::Ptr m_hostsConnector;
            HostsHeartbeatService::Ptr m_hostsHeartbeatService;

            std::mutex x_modulePacketHandler;
            std::map<protocol::ModuleID, ModulePacketHandler> m_modulePacketHandler;

            // 路由选择算法-距离矢量
            DistanceVector::Ptr m_distanceVector;
        };

    } // server

}

#endif //TCPNETWORK_SERVICECONFIG_H
