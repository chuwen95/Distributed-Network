//
// Created by ChuWen on 2024/6/12.
//

#ifndef CLUSTERPEERSERVER_H
#define CLUSTERPEERSERVER_H

#include "ClusterNode.h"

namespace csm
{

    namespace consensus
    {

        class ClusterPeerNode : public ClusterNode
        {
        public:
            ClusterPeerNode() = default;
            ~ClusterPeerNode() = default;
        };

    }

}

#endif //CLUSTERPEERSERVER_H
