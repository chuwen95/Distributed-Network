//
// Created by ChuWen on 9/5/23.
//

#ifndef COPYSTATEMACHINE_P2PSERVICE_H
#define COPYSTATEMACHINE_P2PSERVICE_H

#include "csm-service/config/P2PServiceConfig.h"

namespace csm
{

    namespace service
    {
        // 作为节点启动，需要service模块全内容和host模块，节点相互连接
        class P2PService
        {
        public:
            explicit P2PService(std::unique_ptr<P2PServiceConfig> nodeP2PServiceConfig);

        public:
            int init();

            int start();

            int stop();

            void registerModulePacketHandler(protocol::ModuleID moduleId, ModulePacketHandler packetHander);

            int boardcastModuleMessage(protocol::ModuleID moduleId, const std::vector<char>& data);

            int sendModuleMessageByNodeId(const NodeId& nodeId, protocol::ModuleID moduleId, const std::vector<char>& data);

        private:
            int initServer();

            int initClient();

            int initDistanceVector();

            P2PSession::Ptr getP2PSession(SessionId sessionId);

            int sendData(std::uint64_t fd, const std::vector<char>& data);

            /**
             * 断开客户端
             * @param hostEndPointInfo
             * @param id
             * @param uuid
             * @param flag  0：因为socket发生错误而断开，-1/-2/-3为协议错误码
             * @return
             */
            void disconnectClient(const HostEndPointInfo& hostEndPointInfo, const std::string& id,
                                 int flag, const SessionId& sessionId);

        private:
            int m_fd{-1};

            std::map<std::int32_t, std::function<int(std::shared_ptr<std::vector<char>>)>> m_modulePacketHandler;

            std::unique_ptr<P2PServiceConfig> m_serviceConfig;
        };

    }

}

#endif //COPYSTATEMACHINE_NODEP2PSERVICE_H
