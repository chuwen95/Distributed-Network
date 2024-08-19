//
// Created by ChuWen on 2024/3/23.
//

#include "RocksDBStorage.h"

#include "csm-utilities/Logger.h"
#include "rocksdb/convenience.h"

using namespace csm::storage;

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
    rocksdb::Status status;

    // 创建列族
    rocksdb::ColumnFamilyHandle* cfHandle{ nullptr };
    status = m_db->CreateColumnFamily(rocksdb::ColumnFamilyOptions(), tableName, &cfHandle);
    if(false == status.ok())
    {
        LOG->write(utilities::LogType::Log_Error, FILE_INFO,
                   "RocksDB get error, ercode: ", status.code(), ", message: ", status.getState());
        return -1;
    }

    status = m_db->Get(rocksdb::ReadOptions(), cfHandle, key, &value);
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
    rocksdb::Status status;

    // 创建列族
    rocksdb::ColumnFamilyHandle* cfHandle{ nullptr };
    status = m_db->CreateColumnFamily(rocksdb::ColumnFamilyOptions(), tableName, &cfHandle);
    if(false == status.ok())
    {
        LOG->write(utilities::LogType::Log_Error, FILE_INFO,
                   "RocksDB get error, ercode: ", status.code(), ", message: ", status.getState());
        return -1;
    }

    status = m_db->Put(rocksdb::WriteOptions(), cfHandle, key, rocksdb::Slice(value.data(), value.size()));
    if(false == status.ok())
    {
        LOG->write(utilities::LogType::Log_Error, FILE_INFO,
                   "RocksDB get error, ercode: ", status.code(), ", message: ", status.getState());
        return -1;
    }

    return 0;
}
