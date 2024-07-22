//
// Created by ChuWen on 2024/6/4.
//

#ifndef COPYSTATEMACHINE_RAFTFACTORY_H
#define COPYSTATEMACHINE_RAFTFACTORY_H

#include "csm-common/Common.h"
#include "csm-consensus/csm-raft/Raft.h"

namespace csm
{

    namespace consensus
    {

        class RaftFactory
        {
        public:
            using Ptr = std::shared_ptr<RaftFactory>;

            explicit RaftFactory(const std::vector<std::string>& clusterServerIds);
            ~RaftFactory() = default;

        public:
            Raft::Ptr createRaft();

        private:
            std::vector<std::string> m_clusterServerIds;
        };

    };

}


#endif //COPYSTATEMACHINE_RAFTFACTORY_H
