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
            int init();

            int start();

            int stop();

        public:
            /**
             * 日志起始index
             * @return
             */
            std::uint64_t beginIndex();

            /**
             * 日志结束index，日志范围为[beginIndex, endIndex]
             * @return
             */
            std::uint64_t endIndex();

            /**
             * 第一个日志一定是集群配置日志
             * @return
             */
            int buildInitialConfigurationLog();

            /**
             * 获取集群节点ID
             * @return
             */
            const std::vector<std::string>& clusterServerIds();

        private:
            /**
             * 日志结束index + 1
             * @return
             */
            int increaseEndIndex();

        private:
            StateMachineLogConfig::Ptr m_stateMachineLogConfig;

            std::atomic_uint64_t m_endIndex{ 0 };
        };

    }

}

#endif // STATEMACHINELOG_H
