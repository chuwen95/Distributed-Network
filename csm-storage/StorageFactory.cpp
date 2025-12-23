//
// Created by ChuWen on 2024/7/20.
//

#include "StorageFactory.h"
#include "RocksDBStorage.h"
#include "csm-utilities/Logger.h"

using namespace csm::storage;

StorageFactory::StorageFactory(const StorageType storageType, const std::string &storagePath) :
    m_storageType(storageType),
    m_storagePath(storagePath)
{ }

Storage::Ptr StorageFactory::createStorage()
{
    Storage::Ptr storage{ nullptr };

    if(StorageType::RocksDB == m_storageType)
    {
        Storage::Ptr storage = std::make_shared<RocksDBStorage>(m_storagePath);
    }
    else
    {
        LOG->write(csm::utilities::LogType::Log_Error, FILE_INFO, "unknown storage type: ", m_storageType);
        return nullptr;
    }
    
    int ret = storage->init();
    if(0 != ret)
    {
        LOG->write(csm::utilities::LogType::Log_Error, FILE_INFO, "storage init failed");
        return nullptr;
    }

    return storage;
}