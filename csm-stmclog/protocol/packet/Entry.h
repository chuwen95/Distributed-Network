//
// Created by ChuWen on 2024/6/22.
//

#ifndef ENTRY_H
#define ENTRY_H

#include "protocol/pb/entry.pb.h"

namespace csm
{

    namespace stmclog
    {

        enum class EntryType
        {
            Unknown,
            Configuration,      // 集群配置，在共识模块内部使用
            Data,                   // 数据，需要被状态机执行
            NoOperation,
        };

        class Entry
        {
        public:
            using Ptr = std::shared_ptr<Entry>;

            Entry() = default;
            ~Entry() = default;

        public:
            std::uint64_t index() const;
            void setIndex(const std::uint64_t index);

            std::uint64_t term() const;
            void setTerm(const std::uint64_t term);

            EntryType entryType() const;
            void setEntryType(const EntryType& entryType);

            const std::vector<char> data();
            void setData(const std::vector<char>& data);
            void setData(const std::string& data);

            std::vector<char> encode();
            void decode(const std::vector<char>& data);

        private:
            // 日志条目类型
            EntryType m_type{ EntryType::Unknown };

            stmclog_proto::RawEntry m_rawEntry;
        };

    }

}

#endif //ENTRY_H
