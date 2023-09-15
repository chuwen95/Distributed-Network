//
// Created by root on 9/5/23.
//

#ifndef TCPSERVER_TCPSERVER_H
#define TCPSERVER_TCPSERVER_H

#include "libcommon/Common.h"
#include "libcomponents/SelectListenner.h"
#include "Acceptor.h"
#include "SlaveReactorManager.h"
#include "libpacketprocess/PacketProcessor.h"

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

        void registerDisconnectHandler(std::function<void(const int id)> disconnectHandler);

        int sendData(const int id, const char* data, const std::size_t size);

    private:
        std::string m_ip;
        unsigned short m_port;

        int m_fd;

        components::SelectListenner m_selectListenner;
        Acceptor m_acceptor;
        SlaveReactorManager m_slaveReactorManager;
        packetprocess::PacketProcessor m_packetProcessor;
    };

}




#endif //TCPSERVER_TCPSERVER_H
