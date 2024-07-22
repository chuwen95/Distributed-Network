//
// Created by ChuWen on 2024/6/12.
//

#ifndef CLUSTERPEERSERVER_H
#define CLUSTERPEERSERVER_H

#include "ClusterServer.h"

namespace csm
{

    namespace consensus
    {

        class ClusterPeerServer : public ClusterServer
        {
        public:
            ClusterPeerServer() = default;
            ~ClusterPeerServer() = default;
        };

    }

}

#endif //CLUSTERPEERSERVER_H
