//
// Created by chu on 3/17/25.
//

#ifndef CLUSTERCONFIGURATIONSERIALIZER_H
#define CLUSTERCONFIGURATIONSERIALIZER_H

#include <memory>

#include "csm-framework/cluster/Common.h"

namespace csm
{

    namespace tool
    {

        class ClusterConfigurationSerializer
        {
        public:
            static std::string serialize(const NodeIds& clusterServers);
            static std::shared_ptr<NodeIds> deserialize(const std::string& json);
        };

    }

}




#endif //CLUSTERCONFIGURATIONSERIALIZER_H
