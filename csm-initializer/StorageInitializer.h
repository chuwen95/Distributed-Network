//
// Created by ChuWen on 2024/3/23.
//

#ifndef COPYSTATEMACHINE_STORAGEINITIALIZER_H
#define COPYSTATEMACHINE_STORAGEINITIALIZER_H

#include "csm-storage/Storage.h"

namespace csm
{

    namespace initializer
    {

        class StorageInitializer
        {
        public:
            using Ptr = std::shared_ptr<StorageInitializer>;

            StorageInitializer(const std::string& path);
            ~StorageInitializer() = default;

        public:
            int init();

            storage::Storage::Ptr storage();

        private:
            std::string m_storagePath;

            storage::Storage::Ptr m_storage;
        };

    }

}




#endif //COPYSTATEMACHINE_STORAGEINITIALIZER_H
