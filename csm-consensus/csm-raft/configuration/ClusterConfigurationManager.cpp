//
// Created by ChuWen on 2024/6/5.
//

#include "ClusterConfigurationManager.h"

using namespace csm::consensus;

void ClusterConfigurationManager::addClusterConfiguration(const std::uint64_t index,
    ClusterConfiguration::Ptr clusterConfiguration)
{
    std::unique_lock<std::mutex> ulock(x_clusterConfigurations);

    m_clusterConfigurations.insert(std::make_pair(index, clusterConfiguration));
}

ClusterConfiguration::Ptr ClusterConfigurationManager::lastConfiguration()
{
    std::unique_lock<std::mutex> ulock(x_clusterConfigurations);

    return m_clusterConfigurations.rbegin()->second;
}
