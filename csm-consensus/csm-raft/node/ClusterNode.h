//
// Created by ChuWen on 2024/6/12.
//

#ifndef CLUSTERNODE_H
#define CLUSTERNODE_H

#include <memory>

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

        public:
            virtual void setIsVote(bool isVote) = 0;
            virtual bool isVote() const = 0;

        protected:
            bool m_isVote{ false };
        };

    }

}

#endif //CLUSTERNODE_H
