//
// Created by ChuWen on 2024/8/15.
//

#include "ClusterServerFactory.h"

using namespace csm::consensus;

ClusterServer::Ptr ClusterServerFactory::createClusterServer(const ClusterServerType type)
{
    switch(type)
    {
        case ClusterServerType::Local:
            return std::make_shared<ClusterLocalServer>();
        case ClusterServerType::Peer:
            return std::make_shared<ClusterLocalServer>();
        default:
            return nullptr;
    }
}
