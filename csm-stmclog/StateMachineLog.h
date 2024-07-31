//
// Created by ChuWen on 2024/6/22.
//

#ifndef STATEMACHINELOG_H
#define STATEMACHINELOG_H

#include "csm-common/Common.h"
#include "StateMachineLogConfig.h"

namespace csm
{

    namespace stmclog
    {

        class StateMachineLog
        {
        public:
            using Ptr = std::shared_ptr<StateMachineLog>;

            explicit StateMachineLog(StateMachineLogConfig::Ptr stateMachineLogConfig);
            ~StateMachineLog() = default;

        public:
            /**
             * 读取配置
             * @return
             */
            int init();

            int start();

            int stop();

        private:
            StateMachineLogConfig::Ptr m_stateMachineLogConfig;
        };

    }

}

#endif // STATEMACHINELOG_H
