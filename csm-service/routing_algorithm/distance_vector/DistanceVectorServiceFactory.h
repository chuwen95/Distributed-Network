//
// Created by ivy on 4/20/26.
//

#ifndef COPYSTATEMACHINE_DISTANCEVECTORFACTORY_H
#define COPYSTATEMACHINE_DISTANCEVECTORFACTORY_H

#include "DistanceVectorServiceInterface.h"

namespace csm
{

    namespace service
    {

        class DistanceVectorServiceFactory
        {
        public:
            static std::unique_ptr<DistanceVectorServiceInterface> create(NodeId selfNodeId, NodeIds nodeIds);
        };

    }

}

#endif //COPYSTATEMACHINE_DISTANCEVECTORFACTORY_H