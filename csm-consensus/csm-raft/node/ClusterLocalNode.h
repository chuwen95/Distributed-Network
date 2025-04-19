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
            using Ptr = std::shared_ptr<ClusterLocalNode>;

            ClusterLocalNode() = default;
            ~ClusterLocalNode() override = default;

        public:
            void setIsVote(const bool isVote) override;
            bool isVote() const override;


        };

    }

}

#endif //CLUSTERLOCALNODE_H
