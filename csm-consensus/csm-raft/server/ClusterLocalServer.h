//
// Created by ChuWen on 2024/6/12.
//

#ifndef CLUSTERLOCALSERVER_H
#define CLUSTERLOCALSERVER_H

#include "ClusterServer.h"

namespace csm
{

    namespace consensus
    {

        class ClusterLocalServer : public ClusterServer
        {
        public:
            ClusterLocalServer() = default;
            ~ClusterLocalServer() = default;
        };

    }

}

#endif //CLUSTERLOCALSERVER_H
