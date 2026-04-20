//
// Created by ivy on 4/20/26.
//

#include "DistanceVectorServiceFactory.h"

#include "DistanceVectorService.h"

using namespace csm::service;

std::unique_ptr<DistanceVectorServiceInterface> DistanceVectorServiceFactory::create(NodeId selfNodeId, NodeIds nodeIds)
{
    return std::make_unique<DistanceVectorService>(std::move(selfNodeId), std::move(nodeIds));
}