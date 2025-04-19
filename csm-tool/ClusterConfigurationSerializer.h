//
// Created by chu on 3/17/25.
//

#ifndef CLUSTERCONFIGURATIONSERIALIZER_H
#define CLUSTERCONFIGURATIONSERIALIZER_H

#include "csm-common/Common.h"
#include "csm-framework/consensus/raft/Common.h"

namespace csm
{

    namespace tool
    {

        class ClusterConfigurationSerializer
        {
        public:
            using Ptr = std::shared_ptr<ClusterConfigurationSerializer>;

            ClusterConfigurationSerializer() = default;
            ~ClusterConfigurationSerializer() = default;

        public:
            static std::string serialize(const consensus::NodeIds& clusterServers);
            static std::shared_ptr<consensus::NodeIds> deserialize(const std::string& json);
        };

    }

}




#endif //CLUSTERCONFIGURATIONSERIALIZER_H
