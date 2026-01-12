//
// Created by ChuWen on 2024/7/20.
//

#ifndef COPYSTATEMACHINE_STATEMACHINELOGCONFIG_H
#define COPYSTATEMACHINE_STATEMACHINELOGCONFIG_H

#include "csm-tool/NodeConfig.h"
#include "csm-storage/Storage.h"

namespace csm
{

    namespace stmclog
    {

        class StateMachineLogConfig
        {
        public:
            using Ptr = std::shared_ptr<StateMachineLogConfig>;

            explicit StateMachineLogConfig(tool::NodeConfig::Ptr nodeConfig, storage::Storage::Ptr storage);
            ~StateMachineLogConfig() = default;

        public:
            tool::NodeConfig::Ptr nodeConfig();
            storage::Storage::Ptr storage();

        private:
            tool::NodeConfig::Ptr m_nodeConfig;
            storage::Storage::Ptr m_storage;
        };

    }

}

#endif