//
// Created by ChuWen on 2024/6/5.
//

#ifndef CLUSTERCONFIGURATIONMANAGER_H
#define CLUSTERCONFIGURATIONMANAGER_H

#include "ClusterConfiguration.h"

namespace csm
{

    namespace consensus
    {

        class ClusterConfigurationManager
        {
        public:
            using Ptr = std::shared_ptr<ClusterConfigurationManager>;

            ClusterConfigurationManager() = default;
            ~ClusterConfigurationManager() = default;

        public:
            void addClusterConfiguration(const std::uint64_t index, ClusterConfiguration::Ptr clusterConfiguration);

            ClusterConfiguration::Ptr lastConfiguration();

        private:
            std::mutex x_clusterConfigurations;
            std::map<std::uint64_t, ClusterConfiguration::Ptr> m_clusterConfigurations;
        };

    }

}

#endif //CLUSTERCONFIGMANAGER_H
