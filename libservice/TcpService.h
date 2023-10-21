//
// Created by root on 9/5/23.
//

#ifndef TCPSERVER_TCPSERVER_H
#define TCPSERVER_TCPSERVER_H

#include "libcommon/Common.h"
#include "libcomponents/SelectListenner.h"
#include "libcomponents/ThreadPool.h"
#include "config/ServiceConfig.h"
#include "service/Acceptor.h"
#include "service/SlaveReactorManager.h"
#include "host/HostsInfoManager.h"
#include "host/HostsConnector.h"
#include "host/HostsHeartbeatService.h"

#include "libpacketprocess/packet/PacketBase.h"
#include "libpacketprocess/packet/PacketReplyBase.h"

template<typename T>
concept PacketPtr = requires(T t, char* data, std::size_t size)
{
    t.get();
    { t.use_count() } -> std::same_as<std::size_t>;
    { t.unique() } -> std::same_as<bool>;

    { t->encode(data, size) } -> std::same_as<int>;
};

template<typename T>
concept Packet = requires(T t, char* data, std::size_t size)
{
    { t.encode(data, size) } -> std::same_as<int>;
};

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

        void registerPacketHandler(std::function<int(const packetprocess::PacketType, packetprocess::PacketBase::Ptr,
                                                     packetprocess::PacketReplyBase::Ptr)> packetHander);

        int boardcastMessage(const packetprocess::PacketType packetType, Packet auto const& packet)
        {
            // 编码包为待发送数据
            std::vector<char> buffer;

            packetprocess::PacketHeader packetHeader;
            packetHeader.setType(packetType);
            packetHeader.setPayloadLength(packet.packetLength());

            buffer.resize(packetHeader.headerLength() + packet.packetLength());
            packetHeader.encode(buffer.data(), packetHeader.headerLength());
            packet.encode(buffer.data() + packetHeader.headerLength(), packet.packetLength());

            // 发送给所有在线的节点
            std::vector<std::pair<std::string, int>> allOnlineClients = m_serviceConfig->hostsInfoManager()->getAllOnlineClients();
            for(auto& onlineClient : allOnlineClients)
            {
                int ret = m_serviceConfig->slaveReactorManager()->sendData(onlineClient.second, buffer);
                if(0 != ret)
                {
                    components::Singleton<components::Logger>::instance()->write(components::LogType::Log_Error, FILE_INFO,
                                                                                 "send message to ", onlineClient.second, ", failed, ret: ", ret);
                }
                components::Singleton<components::Logger>::instance()->write(components::LogType::Log_Error, FILE_INFO,
                                                                             "send message to ", onlineClient.second, ", successfully");
            }

            return 0;
        }

    private:
        int initServer();
        int initClient();

        int uninitServer();
        int uninitClient();

        int onClientDisconnect(const HostEndPointInfo& hostEndPointInfo, const std::string& id, const std::string& uuid, const int flag);

    private:
        int m_fd;

        std::function<int(const packetprocess::PacketType, packetprocess::PacketBase::Ptr, packetprocess::PacketReplyBase::Ptr)> m_packetHandler;
        std::function<void(const int, const std::string&)> m_disconnectHandler;

        ServiceConfig::Ptr m_serviceConfig;
    };

}

#endif //TCPSERVER_TCPSERVER_H
