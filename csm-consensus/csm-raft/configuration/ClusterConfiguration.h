//
// Created by ChuWen on 2024/6/5.
//

#ifndef CLUSTERCONFIGURATION_H
#define CLUSTERCONFIGURATION_H

#include "csm-common/Common.h"
#include "csm-consensus/csm-raft/server/ClusterServer.h"

namespace csm
{

    namespace consensus
    {

        class ClusterConfiguration
        {
        public:
            enum class State
            {
                BLANK,                  // 配置未指定任何服务器，对于新加入集群且配置为空的服务器，它们将处于这种状态
                STABLE,                 // 配置指定了一个服务器列表，此列表中的任何多数节点构成法定节点数量
                STAGING,                // 配置指定了两个服务器列表，法定人数是第一个列表中的大多数，但是第二个列表中的服务器也接收日志条目
                TRANSITIONAL        // 配置制定了两个服务器列表，法定人数是第一个列表中的大多数+第二个列表中的大多数
            };

            struct ConfigurationInfo
            {
                std::vector<std::string> serverIds;
                std::map<std::string, ClusterServer::Ptr> servers;
            private:
                std::mutex m;
            };

        public:
            using Ptr = std::shared_ptr<ClusterConfiguration>;

            ClusterConfiguration() = default;
            ~ClusterConfiguration() = default;

        public:
            void setClusterServerIds(const std::vector<std::string>& serverIds);

            void setClusterServerById(const std::string& id, ClusterServer::Ptr clusterServer);
            ClusterServer::Ptr getClusterServerById(const std::string& id);

        private:
            ConfigurationInfo m_oldServers;
            ConfigurationInfo m_newServers;
        };
    }

}




#endif //CLUSTERCONFIG_H
