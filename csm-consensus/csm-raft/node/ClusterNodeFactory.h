//
// Created by ChuWen on 2024/8/15.
//

#ifndef COPYSTATEMACHINE_CLUSTERSERVERFACTORY_H
#define COPYSTATEMACHINE_CLUSTERSERVERFACTORY_H

#include "ClusterLocalNode.h"
#include "ClusterPeerNode.h"

namespace csm
{

    namespace consensus
    {

        enum class ClusterServerType
        {
            Local,
            Peer
        };

        class ClusterNodeFactory
        {
        public:
            using Ptr = std::shared_ptr<ClusterNodeFactory>;

            ClusterNodeFactory() = default;
            ~ClusterNodeFactory() = default;

        public:
            ClusterNode::Ptr createClusterServer(const ClusterServerType type);
        };

    }

}

#endif //COPYSTATEMACHINE_CLUSTERSERVERFACTORY_H
