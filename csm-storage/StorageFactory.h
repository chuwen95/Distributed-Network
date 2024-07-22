//
// Created by ChuWen on 2024/7/20.
//

#ifndef COPYSTATEMACHINE_STORAGEFACTORY_H
#define COPYSTATEMACHINE_STORAGEFACTORY_H

#include "csm-common/Common.h"
#include "csm-storage/Storage.h"

namespace csm
{

    namespace storage
    {

        class StorageFactory
        {
        public:
            using Ptr = std::shared_ptr<StorageFactory>;

            explicit StorageFactory(const StorageType type, const std::string& storagePath);
            ~StorageFactory() = default;

        public:
            Storage::Ptr createStorage();

        private:
            StorageType m_storageType;
            std::string m_storagePath;
        };

    }

}


#endif //COPYSTATEMACHINE_STORAGEFACTORY_H
