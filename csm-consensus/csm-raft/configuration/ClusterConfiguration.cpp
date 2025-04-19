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

void ClusterConfiguration::setClusterNode(const std::string &id, ClusterNode::Ptr clusterServer)
{
    m_configurationInfo.nodes[id] = clusterServer;
}

ClusterNode::Ptr ClusterConfiguration::clusterNode(const NodeId& id)
{
    auto iter = m_configurationInfo.nodes.find(id);
    if (m_configurationInfo.nodes.end() == iter)
    {
        return nullptr;
    }

    return iter->second;
}
