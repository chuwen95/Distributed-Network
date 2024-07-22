//
// Created by ChuWen on 2024/7/20.
//

#ifndef COPYSTATEMACHINE_STORAGE_H
#define COPYSTATEMACHINE_STORAGE_H

#include "csm-common/Common.h"

namespace csm
{

    namespace storage
    {

        enum class StorageType
        {
            RocksDB
        };

        class Storage
        {
        public:
            using Ptr = std::shared_ptr<Storage>;

            Storage() = default;
            virtual ~Storage() = default;

        public:
            virtual int init() = 0;

            /**
             * @brief 从数据库中读取
             *
             * @param table         [in] 表名
             * @param key           [in] 键
             * @param value        [in] 值
             * @return
             */
            virtual int get(const std::string& tableName, const std::string& key, std::string& value) = 0;

            /**
             * @brief 写入数据库
             *
             * @param table         [in] 表名
             * @param key            [in] 键
             * @param value         [in] 值
             * @return
             */
            virtual int set(const std::string& tableName, const std::string& key, const std::vector<char>& value) = 0;
        };

    }

}

#endif //COPYSTATEMACHINE_STORAGE_H
