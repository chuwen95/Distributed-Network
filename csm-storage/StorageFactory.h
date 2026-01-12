//
// Created by ChuWen on 2024/7/20.
//

#ifndef COPYSTATEMACHINE_STORAGEFACTORY_H
#define COPYSTATEMACHINE_STORAGEFACTORY_H

#include <memory>

#include "csm-storage/Storage.h"

namespace csm
{

    namespace storage
    {

        class StorageFactory
        {
        public:
            explicit StorageFactory(StorageType type, const std::string& storagePath);
            ~StorageFactory() = default;

        public:
            std::unique_ptr<Storage> createStorage();

        private:
            StorageType m_storageType;
            std::string m_storagePath;
        };

    }

}


#endif //COPYSTATEMACHINE_STORAGEFACTORY_H
