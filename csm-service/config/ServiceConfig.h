//
// Created by ChuWen on 9/20/23.
//

#ifndef TCPNETWORK_SERVICECONFIG_H
#define TCPNETWORK_SERVICECONFIG_H

#include "csm-common/Common.h"
#include "inipp.h"
#include "csm-tool/NodeConfig.h"

#include "csm-utilities/SelectListenner.h"
#include "csm-service/service/Acceptor.h"
#include "csm-service/service/SessionDispatcher.h"
#include "csm-service/service/SessionDestroyer.h"
#include "csm-service/service/SessionDataProcessor.h"
#include "csm-utilities/ThreadPool.h"

#include "csm-service/host/HostsInfoManager.h"
#include "csm-service/host/HostsConnector.h"
#include "csm-service/host/HostsHeartbeatService.h"

#include "csm-service/service/P2PSessionManager.h"
#include "csm-service/service/ClientAliveChecker.h"

#include "csm-framework/protocol/Protocol.h"

namespace csm
{

    namespace service
    {

        using ModulePacketHandler = std::function<int(std::shared_ptr<std::vector<char>>)>;

        enum class ServiceStartType
        {
            Node,   // 作为节点启动，需要service模块全内容和host模块，节点相互连接
            Server,     // 作为服务器启动，为Rpc模块所用，仅需要service模块部分功能（不包含握手协议部分）
        };

        class ServiceConfig
        {
        public:
            using Ptr = std::shared_ptr<ServiceConfig>;

            ServiceConfig(tool::NodeConfig::Ptr nodeConfig, utilities::SelectListenner::Ptr listenner, Acceptor::Ptr acceptor,
                          P2PSessionManager::Ptr p2pSessionManager, ClientAliveChecker::Ptr clientAliveChecker, std::vector<SlaveReactor::Ptr> slaveReactors,
                          SessionDispatcher::Ptr sessionDispatcher, SessionDestroyer::Ptr sessionDestroyer, SessionDataProcessor::Ptr sessionDataProcessor,
                          const ServiceStartType serverStartType = ServiceStartType::Node, HostsInfoManager::Ptr hostsInfoManager = nullptr,
                          HostsConnector::Ptr hostsConnector = nullptr, HostsHeartbeatService::Ptr hostsHeartbeatService = nullptr);
            ~ServiceConfig() = default;

        public:
            tool::NodeConfig::Ptr nodeConfig();

            utilities::SelectListenner::Ptr listenner();
            Acceptor::Ptr acceptor();

            P2PSessionManager::Ptr p2pSessionManager();
            ClientAliveChecker::Ptr clientAliveChecker();

            std::vector<SlaveReactor::Ptr>& slaveReactors();
            SessionDispatcher::Ptr sessionDispatcher();
            SessionDestroyer::Ptr sessionDestroyer();
            SessionDataProcessor::Ptr sessionDataProcessor();

            ServiceStartType serviceStartType() const;

            HostsInfoManager::Ptr hostsInfoManager();
            HostsConnector::Ptr hostsConnector();
            HostsHeartbeatService::Ptr hostsHeartbeatService();

            void registerModulePacketHandler(protocol::ModuleID moduleId, ModulePacketHandler modulePacketHandler);
            ModulePacketHandler getModulePacketHandler(protocol::ModuleID moduleId);

        private:
            tool::NodeConfig::Ptr m_nodeConfig;

            utilities::SelectListenner::Ptr m_listenner;
            Acceptor::Ptr m_acceptor;

            P2PSessionManager::Ptr m_p2pSessionManager;
            ClientAliveChecker::Ptr m_clientAliveChecker;
            std::vector<SlaveReactor::Ptr> m_slaveReactors;

            SessionDispatcher::Ptr m_sessionDispatcher;
            SessionDestroyer::Ptr m_sessionDestroyer;
            SessionDataProcessor::Ptr m_sessionDataProcessor;
            utilities::ThreadPool::Ptr m_packetProcesser;

            ServiceStartType m_serviceStartType;

            HostsInfoManager::Ptr m_hostsInfoManager;
            HostsConnector::Ptr m_hostsConnector;
            HostsHeartbeatService::Ptr m_hostsHeartbeatService;

            std::mutex x_modulePacketHandler;
            std::map<protocol::ModuleID, std::function<int(std::shared_ptr<std::vector<char>>)>> m_modulePacketHandler;
        };

    } // server

}

#endif //TCPNETWORK_SERVICECONFIG_H
