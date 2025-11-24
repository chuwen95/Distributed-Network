//
// Created by ChuWen on 10/11/23.
//

#ifndef TCPNETWORK_NODECONFIG_H
#define TCPNETWORK_NODECONFIG_H

#include "csm-framework/cluster/Common.h"
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

            explicit NodeConfig(std::string configFile);

        public:
            int init();

        public:
            std::string configFile() const;

            // [cluster]
            // 本节点的id
            NodeId nodeId() const;
            // 集群中所有的服务器id
            const NodeIds& clusterServerIds() const;

            // [raft]
            std::uint32_t minRandomVoteTimeout() const;
            std::uint32_t maxRandomVoteTimeout() const;

            // [rpc]
            // 接收客户端连接的http rpc模块的监听地址
            std::string httpRpcIp() const;
            // 接收客户端连接的http rpc模块的监听端口
            unsigned short httpRpcPort() const;
            // 接收客户端连接的tcp rpc模块的监听地址
            std::string tcpRpcIp() const;
            // 接收客户端连接的tcp rpc模块的监听端口
            unsigned short tcpRpcPort() const;

            // [network]
            // 节点间通信p2p监听地址
            std::string p2pIp() const;
            // 节点间通信p2p监听端口
            unsigned short p2pPort() const;
            // 存放所有节点地址的文件
            std::string nodesFile() const;
            // 从reactor数量
            std::size_t slaveReactorNum() const;
            // 每连接一定数量的客户端就重新检查一下哪个reactor所管理的客户端数量最少
            // 下一轮将新上线的客户端分配给该reactor管理
            std::size_t redispatchInterval() const;
            // 包解码线程池线程数量
            std::size_t sessionDataDecoderWorkerNum() const;
            // 包处理线程池线程数量
            std::size_t moduleDataProcessWorkerNum() const;
            // 路由选择算法
            std::string routingAlgorithm() const;

            // [log]
            // 是否将日志写入到文件
            bool enableFileLog() const;
            // 是否开启日志控制台输出
            bool consoleOutput() const;
            // 当前日志级别
            csm::utilities::LogType logType() const;
            // 日志文件位置
            std::string logPath() const;

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
            NodeId m_nodeId;
            NodeIds m_clusterServerIds;

            // [raft]
            std::uint32_t m_minRandomVoteTimeout{150};
            std::uint32_t m_maxRandomVoteTimeout{300};

            // [rpc]
            std::string m_httpRpcIp{ "127.0.0.1" };
            unsigned short m_httpRpcPort{ 8200 };
            std::string m_tcpRpcIp{ "127.0.0.1" };
            unsigned short m_tcpRpcPort{ 20200 };

            // [network]
            std::string m_p2pIp{ "127.0.0.1" };
            unsigned short m_p2pPort{ 30200 };
            std::string m_nodesFile{ "nodes.json" };
            std::size_t m_sessionDataDecodeWorkerNum{ 4 };
            std::size_t m_moduleDataProcessWorkerNum{ 8 };
            std::string m_routingAlgorithm{ "distance_vector" };

            // [reactor]
            std::size_t m_slaveReactorNum{ 8 };
            std::size_t m_redispatchInterval{ 50 };

            // []
            std::size_t m_packetProcessThreadNum{ 8 };

            // [log]
            bool m_enableFileLog{ true };
            bool m_consoleOutput{ false };
            csm::utilities::LogType m_logType{ csm::utilities::LogType::Log_Info };
            std::string m_logPath{ "./logs" };
        };

    } // tool

}

#endif //TCPNETWORK_NODECONFIG_H
