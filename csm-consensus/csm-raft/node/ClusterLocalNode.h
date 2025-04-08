//
// Created by ChuWen on 2024/6/12.
//

#ifndef CLUSTERLOCALNODE_H
#define CLUSTERLOCALNODE_H

#include "ClusterNode.h"

namespace csm
{

    namespace consensus
    {

        class ClusterLocalNode : public ClusterNode
        {
        public:
            ClusterLocalNode() = default;
            ~ClusterLocalNode() = default;
        };

    }

}

#endif //CLUSTERLOCALNODE_H
