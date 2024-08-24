//
// Created by ChuWen on 10/11/23.
//

#ifndef TCPNETWORK_NODECONFIG_H
#define TCPNETWORK_NODECONFIG_H

#include "csm-common/Common.h"
#include "csm-utilities/Logger.h"
#include "inipp.h"

namespace csm
{

    namespace tool
    {

        class NodeConfig
        {
        public:
            using Ptr = std::shared_ptr<NodeConfig>;

            NodeConfig() = default;
            ~NodeConfig() = default;

        public:
            int init(const std::string &configFile);

        public:
            std::string configFile();

            // [cluster]
            // 本节点的id
            std::string id();
            // 集群中所有的服务器id
            const std::vector<std::string>& clusterServerIds() const;

            // [rpc]
            // 接收客户端连接的rpc模块的监听地址
            std::string rpcIp();
            // 接收客户端连接的rpc模块的监听端口
            unsigned short rpcPort();

            // [network]
            // 节点间通信p2p监听地址
            std::string p2pIp();
            // 节点间通信p2p监听端口
            unsigned short p2pPort();
            // 存放所有节点地址的文件
            std::string nodesFile();
            // 从reactor数量
            std::size_t slaveReactorNum();
            // 每连接一定数量的客户端就重新检查一下哪个reactor所管理的客户端数量最少
            // 下一轮将新上线的客户端分配给该reactor管理
            std::size_t redispatchInterval();
            // 包处理线程池线程数量
            std::size_t sessionDataWorkerNum();
            // 仅作为客户端启动
            bool startAsClient();

            // [log]
            // 是否将日志写入到文件
            bool enableFileLog();
            // 是否开启日志控制台输出
            bool consoleOutput();
            // 当前日志级别
            csm::utilities::LogType logType();
            // 日志文件位置
            std::string logPath();

        public:
            int loadLogConfig();

        private:
            // 解析集群配置
            int parseClusterConfig(inipp::Ini<char> &ini);
            // 解析RPC配置
            int parseRpcConfig(inipp::Ini<char> &ini);
            // 解析网络配置
            int parseNetworkConfig(inipp::Ini<char> &ini);
            // 解析日志配置
            int parseLogConfig(inipp::Ini<char> &ini);

        private:
            std::string m_configFile;
            // log absolute dir path
            std::string m_configDir;

            // [info]
            std::string m_id;
            std::vector<std::string> m_clusterServerIds;

            // [feature]
            bool m_startAsClient{false};

            // [rpc]
            std::string m_rpcIp{"127.0.0.1"};
            unsigned short m_rpcPort{20200};

            // [network]
            std::string m_p2pIp{"127.0.0.1"};
            unsigned short m_p2pPort{30200};
            std::string m_nodesFile{"nodes.json"};
            std::size_t m_sessionDataWorkerNum{8};

            // [reactor]
            std::size_t m_slaveReactorNum{8};
            std::size_t m_redispatchInterval{50};

            // []
            std::size_t m_packetProcessThreadNum{8};

            // [log]
            bool m_enableFileLog{true};
            bool m_consoleOutput{true};
            csm::utilities::LogType m_logType{csm::utilities::LogType::Log_Info};
            std::string m_logPath{"./logs"};
        };

    } // tool

}

#endif //TCPNETWORK_NODECONFIG_H
