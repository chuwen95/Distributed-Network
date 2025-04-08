//
// Created by chu on 3/18/25.
//

#ifndef VOLATILESTATE_H
#define VOLATILESTATE_H

#include "csm-common/Common.h"

namespace csm
{

    namespace consensus
    {
        /**
         * 所有服务器的易失状态
         */
        class VolatileState
        {
        public:
            using Ptr = std::shared_ptr<VolatileState>;

            VolatileState() = default;
            ~VolatileState() = default;

            std::uint64_t commitIndex() const;
            std::uint64_t lastAppliedIndex() const;

        private:
            std::atomic_uint64_t m_commitIndex{ 0 };    // 已知已提交的最高日志条目的索引（初始化为0,单调递增）
            std::atomic_uint64_t m_lastAppliedIndex{ 0 };   // 应用于状态机的最高日志的索引（初始化为0,单调递增）
        };

    }

}



#endif //VOLATILESTATE_H
