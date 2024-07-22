//
// Created by ChuWen on 2024/3/21.
//

#ifndef COPYSTATEMACHINE_ROCKSDBSTORAGE_H
#define COPYSTATEMACHINE_ROCKSDBSTORAGE_H

#include "csm-common/Common.h"
#include "Storage.h"

#include "rocksdb/db.h"
#include "rocksdb/options.h"
#include "rocksdb/slice.h"

namespace csm
{

    namespace storage
    {

        class RocksDBStorage : public Storage
        {
        public:
            using Ptr = std::shared_ptr<RocksDBStorage>;

            RocksDBStorage(const std::string& storagePath);
            ~RocksDBStorage() = default;

        public:
            int init() override;

            /**
             * @brief 从数据库中读取
             *
             * @param table         [in] 表名
             * @param key           [in] 键
             * @param value        [in] 值
             * @return
             */
            int get(const std::string& tableName, const std::string& key, std::string& value) override;

            /**
             * @brief 写入数据库
             *
             * @param table         [in] 表名
             * @param key            [in] 键
             * @param value         [in] 值
             * @return
             */
            int set(const std::string& tableName, const std::string& key, const std::vector<char>& value) override;

        private:
            std::string generateDBKey(const std::string& tableName,  const std::string& key);

        private:
            std::string m_storagePath;
            std::unique_ptr<rocksdb::DB, std::function<void(rocksdb::DB*)>> m_db;
        };

    }

}


#endif //COPYSTATEMACHINE_ROCKSDBSTORAGE_H
