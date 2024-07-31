//
// Created by ChuWen on 2024/7/31.
//

#ifndef COPYSTATEMACHINE_STATEMACHINELOGFACTORY_H
#define COPYSTATEMACHINE_STATEMACHINELOGFACTORY_H

#include "csm-common/Common.h"
#include "csm-storage/Storage.h"
#include "csm-stmclog/StateMachineLog.h"

namespace csm
{

    namespace stmclog
    {

        class StateMachineLogFactory
        {
        public:
            using Ptr = std::shared_ptr<StateMachineLogFactory>;

            explicit StateMachineLogFactory(storage::Storage::Ptr storage);
            ~StateMachineLogFactory() = default;

        public:
            StateMachineLog::Ptr createStateMachineLog();

        private:
            storage::Storage::Ptr m_storage;
        };

    }

}

#endif //COPYSTATEMACHINE_STATEMACHINELOGFACTORY_H
