//
// Created by root on 9/5/23.
//

#ifndef TCPSERVER_TCPSERVER_H
#define TCPSERVER_TCPSERVER_H

#include "csm-common/Common.h"
#include "csm-utilities/SelectListenner.h"
#include "csm-utilities/ThreadPool.h"
#include "config/ServiceConfig.h"
#include "service/Acceptor.h"
#include "service/SlaveReactorManager.h"
#include "host/HostsInfoManager.h"
#include "host/HostsConnector.h"
#include "host/HostsHeartbeatService.h"

namespace csm
{

    namespace service
    {

        class TcpService
        {
        public:
            using Ptr = std::shared_ptr<TcpService>;

            TcpService(ServiceConfig::Ptr serviceConfig);
            ~TcpService() = default;

        public:
            int init();

            int uninit();

            int start();

            int stop();

            void registerModulePacketHandler(
                    const std::int32_t moduleId, std::function<int(std::shared_ptr<std::vector<char>>)> packetHander);

        public:
            /**
             * @brief 仅作为客户端的时候使用，等待至少有一个节点连上
             * @param timeout           [in]超时时间（单位：毫秒）
             *
             * @return 如果有超时时间内有节点上线，则返回true
             *               如果超时时间内没有节点上线，则返回false
             */
            bool waitAtLeastOneNodeConnected(const int timeout);

            int boardcastModuleMessage(const std::int32_t moduleId, std::shared_ptr<std::vector<char>> data);

            int sendModuleMessageByNodeId(
                    const std::string &nodeId, const std::int32_t moduleId, std::shared_ptr<std::vector<char>> data);

        private:
            int initServer();

            int initClient();

            int uninitServer();

            int uninitClient();

            int onClientDisconnect(
                    const HostEndPointInfo &hostEndPointInfo, const std::string &id, const std::string &uuid, const int flag);

        private:
            int m_fd;

            std::map<std::int32_t, std::function<int(std::shared_ptr<std::vector<char>>)>> m_modulePacketHandler;
            std::function<void(const int, const std::string &)> m_disconnectHandler;

            ServiceConfig::Ptr m_serviceConfig;
        };

    }

}

#endif //TCPSERVER_TCPSERVER_H
