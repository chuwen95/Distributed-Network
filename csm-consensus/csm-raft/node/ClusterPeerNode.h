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
            using Ptr = std::shared_ptr<ClusterPeerNode>;

            ClusterPeerNode() = default;
            ~ClusterPeerNode() override = default;

        public:
            void setIsVote(const bool isVote) override;
            bool isVote() const override;
        };

    }

}

#endif //CLUSTERPEERSERVER_H
