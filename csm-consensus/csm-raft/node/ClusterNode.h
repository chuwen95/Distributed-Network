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

        class ClusterNode
        {
        public:
            using Ptr = std::shared_ptr<ClusterNode>;

            ClusterNode() = default;
            virtual ~ClusterNode() = default;
        };

    }

}

#endif //SERVER_H
