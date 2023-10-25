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

        void registerModulePacketHandler(const std::int32_t moduleId, std::function<int(std::shared_ptr<std::vector<char>>)> packetHander);

        int boardcastModuleMessage(const std::int32_t moduleId, std::shared_ptr<std::vector<char>> data)
        {
            PacketHeader packetHeader;
            packetHeader.setType(PacketType::PT_ModuleMessage);
            packetHeader.setModuleId(moduleId);
            packetHeader.setPayloadLength(data->size());

            // 编码包为待发送数据
            std::vector<char> buffer;
            buffer.resize(packetHeader.headerLength() + data->size());
            packetHeader.encode(buffer.data(), packetHeader.headerLength());
            memcpy(buffer.data() + packetHeader.headerLength(), data->data(), data->size());

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
                components::Singleton<components::Logger>::instance()->write(components::LogType::Log_Debug, FILE_INFO,
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

        std::map<std::int32_t, std::function<int(std::shared_ptr<std::vector<char>>)>> m_modulePacketHandler;
        std::function<void(const int, const std::string&)> m_disconnectHandler;

        ServiceConfig::Ptr m_serviceConfig;
    };

}

#endif //TCPSERVER_TCPSERVER_H
