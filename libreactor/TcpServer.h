//
// Created by root on 9/5/23.
//

#ifndef TCPSERVER_TCPSERVER_H
#define TCPSERVER_TCPSERVER_H

#include "libcommon/Common.h"
#include "libcomponents/SelectListenner.h"
#include "libcomponents/ThreadPool.h"
#include "Acceptor.h"
#include "SlaveReactorManager.h"

#include "libpacketprocess/packet/PacketBase.h"
#include "libpacketprocess/packet/PacketReplyBase.h"

namespace server
{

    class TcpServer
    {
    public:
        TcpServer() = default;
        ~TcpServer() = default;

    public:
        int init(const std::string_view ip, const unsigned short port, const std::string_view logPath);

        int uninit();

        int start();

        int stop();

        void setLogLevel(const int logLevel);

        void registerPacketHandler(std::function<int(const packetprocess::PacketType, packetprocess::PacketBase::Ptr,
                packetprocess::PacketReplyBase::Ptr)> packetHander);

        void registerDisconnectHandler(std::function<void(const int id, const std::string&)> disconnectHandler);

    private:
        std::string m_ip;
        unsigned short m_port;

        int m_fd;

        std::function<int(const packetprocess::PacketType, packetprocess::PacketBase::Ptr, packetprocess::PacketReplyBase::Ptr)> m_packetHander;
        std::function<void(const int, const std::string&)> m_disconnectHandler;

        components::SelectListenner m_selectListenner;
        Acceptor m_acceptor;
        SlaveReactorManager m_slaveReactorManager;
        components::ThreadPool m_packetProcessThreadPoll;
    };

}




#endif //TCPSERVER_TCPSERVER_H
