//
// Created by ChuWen on 2024/6/22.
//

#ifndef ENTRY_H
#define ENTRY_H

#include "csm-common/Common.h"

namespace csm
{

    namespace consensus
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

        private:
            // 日志条目类型
            EntryType m_type;
            // 日志条目的index
            std::uint64_t m_index;
        };

    }

}

#endif //ENTRY_H
