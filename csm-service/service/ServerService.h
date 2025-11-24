//
// Created by ChuWen on 9/5/23.
//

#ifndef COPYSTATEMACHINE_SERVERSERVER_H
#define COPYSTATEMACHINE_SERVERSERVER_H

#include "csm-service/config/ServerServiceConfig.h"

namespace csm
{

    namespace service
    {
        // 作为服务器启动，为RPC模块所用，仅需要service模块部分功能，不包含握手协议部分
        class ServerService
        {
        public:
            explicit ServerService(std::unique_ptr<ServerServiceConfig> serviceConfig);
            ~ServerService();

        public:
            int init();

            int start();

            int stop();

            void registerPacketHandler(std::function<void(SessionId sessionId, P2PSession::WPtr p2pSessionWeakPtr,
                PacketHeader::Ptr header,
                PayloadBase::Ptr payload)> handler);

        public:
            void setDisconnectHandler(std::function<void(const HostEndPointInfo& hostEndPointInfo, const NodeId& nodeId, int flag,
                const SessionId& sessionId)> handler);

        private:
            int initServer();

        private:
            int m_fd{-1};

            std::function<void(SessionId sessionId, P2PSession::WPtr p2pSessionWeakPtr, PacketHeader::Ptr header,
                PayloadBase::Ptr payload)> m_packetHandler;

            std::function<void(const HostEndPointInfo& hostEndPointInfo, const NodeId& nodeId, int flag,
                const SessionId& sessionId)> m_disconnectHandler;

            std::unique_ptr<ServerServiceConfig> m_serviceConfig;
        };

    }

}

#endif //COPYSTATEMACHINE_SERVERSERVER_H