//
// Created by ChuWen on 2024/6/5.
//

#include "ClusterConfiguration.h"

using namespace csm::consensus;

void ClusterConfiguration::setClusterServerIds(const std::vector<std::string> &serverIds)
{
    m_oldServers.serverIds = serverIds;
}

void ClusterConfiguration::setClusterServerById(const std::string &id, ClusterServer::Ptr clusterServer)
{
    m_oldServers.servers[id] = clusterServer;
}
