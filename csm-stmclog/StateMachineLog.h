//
// Created by ChuWen on 2024/6/22.
//

#ifndef STATEMACHINELOG_H
#define STATEMACHINELOG_H

#include "csm-common/Common.h"

namespace csm
{

    namespace consensus
    {

        class StateMachineLog
        {
        public:
            using Ptr = std::shared_ptr<StateMachineLog>;

            StateMachineLog() = default;
            ~StateMachineLog() = default;

        public:
            /**
             * 读取配置
             * @return
             */
            int init();
        };

    }

}

#endif // STATEMACHINELOG_H
