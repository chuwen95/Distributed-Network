//
// Created by root on 9/20/23.
//

#ifndef TCPNETWORK_SERVERCONFIG_H
#define TCPNETWORK_SERVERCONFIG_H

#include "libcommon/Common.h"
#include "libcomponents/Logger.h"
#include "inipp.h"

namespace server
{

    class ServerConfig
    {
    public:
        ServerConfig() = default;
        ~ServerConfig() = default;

    public:
        int init(const std::string& configFile);

        int uninit();

    public:
        std::string ip();
        unsigned short port();

        std::size_t slaveReactorNum();
        std::size_t redispatchInterval();

        std::size_t packetProcessThreadNum();

        bool enableFileLog();
        bool consoleOutput();
        components::LogType logType();
        std::string logPath();

    private:
        int parseNetworkConfig(inipp::Ini<char>& ini);
        int parseReactorConfig(inipp::Ini<char>& ini);
        int parsePacketProcessConfig(inipp::Ini<char>& ini);
        int parseLogConfig(inipp::Ini<char>& ini);

    private:
        // [network]
        std::string m_ip{"127.0.0.1"};
        unsigned short m_port{9999};

        // [reactor]
        std::size_t m_slaveReactorNum{8};
        std::size_t m_redispatchInterval{50};

        // [packet_process]
        std::size_t m_packetProcessThreadNum{8};

        // [log]
        bool m_enableFileLog{true};
        bool m_consoleOutput{true};
        components::LogType m_logType{components::LogType::Log_Info};
        std::string m_logPath{"./server.log"};
    };

} // server

#endif //TCPNETWORK_SERVERCONFIG_H
