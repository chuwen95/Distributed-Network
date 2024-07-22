//
// Created by ChuWen on 2024/3/23.
//

#include "RocksDBStorage.h"

#include "csm-utilities/Logger.h"
#include "rocksdb/convenience.h"

using namespace csm::storage;

constexpr const char* const c_tableKeySplit{ ":"};

RocksDBStorage::RocksDBStorage(const std::string& storagePath) :
        m_storagePath(storagePath)
{}

int RocksDBStorage::init()
{
    rocksdb::Options options;
    options.IncreaseParallelism();
    options.OptimizeLevelStyleCompaction();
    options.create_if_missing = true;

    rocksdb::DB* db;
    rocksdb::Status status = rocksdb::DB::Open(options, m_storagePath, &db);
    if(false == status.ok())
    {
        LOG->write(utilities::LogType::Log_Error, FILE_INFO,
                   "RocksDB get error, ercode: ", status.code(), ", message: ", status.getState());
        return -1;
    }

    m_db = std::unique_ptr<rocksdb::DB, std::function<void(rocksdb::DB*)>>(db, [](rocksdb::DB* db){
        rocksdb::CancelAllBackgroundWork(db, true);
        delete db;
    });

    return 0;
}

int RocksDBStorage::get(const std::string &tableName, const std::string &key, std::string &value)
{
    std::string dbKey = generateDBKey(tableName, key);

    rocksdb::Status status = m_db->Get(rocksdb::ReadOptions(), key, &value);
    if(false == status.ok())
    {
        LOG->write(utilities::LogType::Log_Error, FILE_INFO,
                   "RocksDB get error, ercode: ", status.code(), ", message: ", status.getState());
        return -1;
    }

    return 0;
}

int RocksDBStorage::set(const std::string &tableName, const std::string &key, const std::vector<char> &value)
{
    std::string dbKey = generateDBKey(tableName, key);

    rocksdb::Status status = m_db->Put(rocksdb::WriteOptions(), key, rocksdb::Slice(value.data(), value.size()));
    if(false == status.ok())
    {
        LOG->write(utilities::LogType::Log_Error, FILE_INFO,
                   "RocksDB get error, ercode: ", status.code(), ", message: ", status.getState());
        return -1;
    }

    return 0;
}

std::string RocksDBStorage::generateDBKey(const std::string &tableName, const std::string &key)
{
    std::string dbKey;
    dbKey.reserve(tableName.size() + 1 + key.size());

    dbKey.append(tableName);
    dbKey.append(c_tableKeySplit);
    dbKey.append(key);

    return dbKey;
}
