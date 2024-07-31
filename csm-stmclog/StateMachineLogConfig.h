//
// Created by ChuWen on 2024/7/20.
//

#ifndef COPYSTATEMACHINE_STATEMACHINELOGCONFIG_H
#define COPYSTATEMACHINE_STATEMACHINELOGCONFIG_H

#include "csm-common/Common.h"
#include "csm-storage/Storage.h"

namespace csm
{

    namespace stmclog
    {

        class StateMachineLogConfig
        {
        public:
            using Ptr = std::shared_ptr<StateMachineLogConfig>;

            explicit StateMachineLogConfig(storage::Storage::Ptr storage);
            ~StateMachineLogConfig() = default;

        public:
            storage::Storage::Ptr storage();

        private:
            storage::Storage::Ptr m_storage;
        };

    }

}

#endif //COPYSTATEMACHINE_STATEMACHINELOGCONFIG_H
