//
// Created by ChuWen on 9/5/23.
//

#ifndef P2PSERVER_H
#define P2PSERVER_H

#include "csm-common/Common.h"
#include "config/ServiceConfig.h"
#include "csm-framework/cluster/Common.h"
#include "service/SessionDispatcher.h"
#include "host/HostsInfoManager.h"
#include "csm-framework/protocol/Protocol.h"

namespace csm
{

    namespace service
    {

        class P2PService
        {
        public:
            using Ptr = std::shared_ptr<P2PService>;

            explicit P2PService(ServiceConfig::Ptr serviceConfig);
            ~P2PService();

        public:
            int init();

            int start();

            int stop();

            void registerModulePacketHandler(const protocol::ModuleID moduleId, ModulePacketHandler packetHander);

        public:
            /**
             * @brief 仅作为客户端的时候使用，等待至少有一个节点连上
             * @param timeout           [in]超时时间（单位：毫秒）
             *
             * @return 如果有超时时间内有节点上线，则返回true
             *               如果超时时间内没有节点上线，则返回false
             */
            bool waitAtLeastOneNodeConnected(int timeout);

            int boardcastModuleMessage(const protocol::ModuleID moduleId, const std::vector<char>& data);

            int sendModuleMessageByNodeId(const NodeId &nodeId, protocol::ModuleID moduleId, const std::vector<char>& data);

        private:
            int initServer();

            int initClient();

            int initPacketHandler();

            SlaveReactor::Ptr getSlaveReactorByFd(const int fd);

            int sendData(const int fd, const std::vector<char>& data);

            /**
             * 断开客户端
             * @param hostEndPointInfo
             * @param id
             * @param uuid
             * @param flag  0：因为socket发生错误而断开，-1/-2/-3为协议错误码
             * @return
             */
            int disconnectClient(const HostEndPointInfo &hostEndPointInfo, const std::string &id, const std::string &uuid, const int flag);

        private:
            int m_fd{ -1 };

            std::map<std::int32_t, std::function<int(std::shared_ptr<std::vector<char>>)>> m_modulePacketHandler;

            ServiceConfig::Ptr m_serviceConfig;
        };

    }

}

#endif //P2PSERVER_H
