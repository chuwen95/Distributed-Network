//
// Created by root on 9/20/23.
//

#ifndef TCPNETWORK_SERVICECONFIG_H
#define TCPNETWORK_SERVICECONFIG_H

#include "libcommon/Common.h"
#include "libcomponents/Logger.h"
#include "inipp.h"

namespace service
{

    class ServiceConfig
    {
    public:
        ServiceConfig() = default;
        ~ServiceConfig() = default;

    public:
        int init(const std::string& configFile);

        int uninit();

    public:
        // [info]
        std::string id();

        // [feature]
        bool startAsClient();

        // [network]
        std::string ip();
        unsigned short port();
        std::string nodesFile();

        // [reactor]
        std::size_t slaveReactorNum();
        std::size_t redispatchInterval();

        // [packet_process]
        std::size_t packetProcessThreadNum();

        // [log]
        bool enableFileLog();
        bool consoleOutput();
        components::LogType logType();
        std::string logPath();

    private:
        int parseInfoConfig(inipp::Ini<char>& ini);
        int parseFeatureConfig(inipp::Ini<char>& ini);
        int parseNetworkConfig(inipp::Ini<char>& ini);
        int parseReactorConfig(inipp::Ini<char>& ini);
        int parsePacketProcessConfig(inipp::Ini<char>& ini);
        int parseLogConfig(inipp::Ini<char>& ini);

    private:
        // [info]
        std::string m_id;

        // [feature]
        bool m_startAsClient{false};

        // [network]
        std::string m_ip{"127.0.0.1"};
        unsigned short m_port{9999};
        std::string m_nodesFile{"nodes.json"};

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

#endif //TCPNETWORK_SERVICECONFIG_H
