//
// Created by root on 10/11/23.
//

#ifndef TCPNETWORK_NODECONFIG_H
#define TCPNETWORK_NODECONFIG_H

#include "libcommon/Common.h"
#include "libcomponents/Logger.h"
#include "inipp.h"

namespace tools
{

    class NodeConfig
    {
    public:
        using Ptr = std::shared_ptr<NodeConfig>;

        NodeConfig() = default;
        ~NodeConfig() = default;

    public:
        int init(const std::string& configFile);

        int uninit();

    public:
        std::string configFile();

        // [info]
        std::string id();

        // [feature]
        bool startAsClient();

        // [rpc]
        std::string rpcIp();
        unsigned short rpcPort();

        // [network]
        std::string p2pIp();
        unsigned short p2pPort();
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

    public:
        int loadLogConfig();

    private:
        int parseInfoConfig(inipp::Ini<char>& ini);
        int parseFeatureConfig(inipp::Ini<char>& ini);
        int parseRpcConfig(inipp::Ini<char>& ini);
        int parseNetworkConfig(inipp::Ini<char>& ini);
        int parseReactorConfig(inipp::Ini<char>& ini);
        int parsePacketProcessConfig(inipp::Ini<char>& ini);
        int parseLogConfig(inipp::Ini<char>& ini);

    private:
        // log file path
        std::string m_configFile;

        // [info]
        std::string m_id;

        // [feature]
        bool m_startAsClient{false};

        // [rpc]
        std::string m_rpcIp{"127.0.0.1"};
        unsigned short m_rpcPort{20200};

        // [network]
        std::string m_p2pIp{"127.0.0.1"};
        unsigned short m_p2pPort{30200};
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

} // tool

#endif //TCPNETWORK_NODECONFIG_H
