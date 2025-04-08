//
// Created by ChuWen on 2024/6/5.
//

#include "ClusterConfiguration.h"

using namespace csm::consensus;

ClusterConfiguration::ClusterConfiguration(const std::vector<NodeId>& nodeIds)
{
    m_configurationInfo.nodeIds = nodeIds;
}

void ClusterConfiguration::setClusterNodeIds(const std::vector<std::string> &nodeIds)
{
    m_configurationInfo.nodeIds = nodeIds;
}

void ClusterConfiguration::setClusterNodeById(const std::string &id, ClusterNode::Ptr clusterServer)
{
    m_configurationInfo.nodes[id] = clusterServer;
}
