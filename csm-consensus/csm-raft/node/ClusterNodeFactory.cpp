//
// Created by ChuWen on 2024/8/15.
//

#include "ClusterNodeFactory.h"

using namespace csm::consensus;

ClusterNode::Ptr ClusterNodeFactory::createClusterServer(const ClusterServerType type)
{
    switch(type)
    {
        case ClusterServerType::Local:
            return std::make_shared<ClusterLocalNode>();
        case ClusterServerType::Peer:
            return std::make_shared<ClusterLocalNode>();
        default:
            return nullptr;
    }
}
