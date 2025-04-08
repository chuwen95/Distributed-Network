//
// Created by chu on 3/30/25.
//

#ifndef RAFTMETADATASTORAGE_H
#define RAFTMETADATASTORAGE_H

#include "csm-common/Common.h"

#include "csm-storage/Storage.h"

namespace csm
{

    namespace consensus
    {

        class RaftMetadataStorage
        {
        public:
            using Ptr = std::shared_ptr<RaftMetadataStorage>;

            RaftMetadataStorage(storage::Storage::Ptr storage);
            ~RaftMetadataStorage() = default;

        public:
            int getCurrentTerm(std::uint64_t& term) const;
            int setCurrentTerm(std::uint64_t term);

        private:
            storage::Storage::Ptr m_storage;
        };

    }

}

#endif //RAFTMETADATASTORAGE_H
