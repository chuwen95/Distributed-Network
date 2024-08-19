//
// Created by ChuWen on 2024/6/5.
//

#ifndef CLUSTERCONFIGURATIONMANAGER_H
#define CLUSTERCONFIGURATIONMANAGER_H

#include "csm-common/Common.h"

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
            void addClusterConfiguration(const std::uint64_t index, )
        };

    }

}

#endif //CLUSTERCONFIGMANAGER_H
