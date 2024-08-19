//
// Created by ChuWen on 2024/8/15.
//

#ifndef COPYSTATEMACHINE_CLUSTERSERVERFACTORY_H
#define COPYSTATEMACHINE_CLUSTERSERVERFACTORY_H

#include "csm-common/Common.h"

#include "ClusterLocalServer.h"
#include "ClusterPeerServer.h"

namespace csm
{

    namespace consensus
    {

        enum class ClusterServerType
        {
            Local,
            Peer
        };

        class ClusterServerFactory
        {
        public:
            using Ptr = std::shared_ptr<ClusterServerFactory>;

            ClusterServerFactory() = default;
            ~ClusterServerFactory() = default;

        public:
            ClusterServer::Ptr createClusterServer(const ClusterServerType type);
        };

    }

}

#endif //COPYSTATEMACHINE_CLUSTERSERVERFACTORY_H
