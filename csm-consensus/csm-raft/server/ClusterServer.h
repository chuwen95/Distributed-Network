//
// Created by ChuWen on 2024/6/12.
//

#ifndef SERVER_H
#define SERVER_H

#include "csm-common/Common.h"

namespace csm
{

    namespace consensus
    {

        class ClusterServer
        {
        public:
            using Ptr = std::shared_ptr<ClusterServer>;

            ClusterServer() = default;
            virtual ~ClusterServer() = default;
        };

    }

}

#endif //SERVER_H
