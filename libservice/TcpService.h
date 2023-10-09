//
// Created by root on 9/5/23.
//

#ifndef TCPSERVER_TCPSERVER_H
#define TCPSERVER_TCPSERVER_H

#include "libcommon/Common.h"
#include "libcomponents/SelectListenner.h"
#include "libcomponents/ThreadPool.h"
#include "ServiceConfig.h"
#include "Acceptor.h"
#include "SlaveReactorManager.h"
#include "HostsInfoManager.h"
#include "HostsConnector.h"
#include "HostsHeartbeatService.h"

#include "libpacketprocess/packet/PacketBase.h"
#include "libpacketprocess/packet/PacketReplyBase.h"

namespace service
{

    class TcpService
    {
    public:
        TcpService() = default;
        ~TcpService() = default;

    public:
        int init(const std::string& config);

        int uninit();

        int start();

        int stop();

        void registerPacketHandler(std::function<int(const packetprocess::PacketType, packetprocess::PacketBase::Ptr,
                packetprocess::PacketReplyBase::Ptr)> packetHander);

    private:
        int initServer();
        int initClient();

        int uninitServer();
        int uninitClient();

        int onClientDisconnect(const HostEndPointInfo& hostEndPointInfo, const std::string& id, const std::string& uuid, const int flag);

    private:
        std::string m_ip;
        unsigned short m_port;
        int m_fd;
        bool m_startAsClient{false};

        std::function<int(const packetprocess::PacketType, packetprocess::PacketBase::Ptr, packetprocess::PacketReplyBase::Ptr)> m_packetHandler;
        std::function<void(const int, const std::string&)> m_disconnectHandler;

        ServiceConfig m_serviceConfig;

        components::SelectListenner m_selectListenner;
        Acceptor m_acceptor;
        SlaveReactorManager m_slaveReactorManager;
        components::ThreadPool m_packetProcessThreadPoll;

        HostsInfoManager::Ptr m_hostsInfoManager;
        HostsConnector m_hostsConnector;
        HostsHeartbeatService m_hostsHeartbeatService;
    };

}




#endif //TCPSERVER_TCPSERVER_H
