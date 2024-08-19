//
// Created by ChuWen on 2024/7/31.
//

#ifndef COPYSTATEMACHINE_STATEMACHINELOGINITIALIZER_H
#define COPYSTATEMACHINE_STATEMACHINELOGINITIALIZER_H

#include "csm-common/Common.h"
#include "csm-tool/NodeConfig.h"
#include "csm-storage/Storage.h"
#include "csm-stmclog/StateMachineLog.h"

namespace csm
{

namespace initializer
{

    class StateMachineLogInitializer
    {
    public:
        using Ptr = std::shared_ptr<StateMachineLogInitializer>;

        explicit StateMachineLogInitializer(tool::NodeConfig::Ptr nodeConfig, storage::Storage::Ptr storage);
        ~StateMachineLogInitializer();

    public:
        int init();

        int start();

        int stop();

    private:
        stmclog::StateMachineLog::Ptr m_stateMachineLog;
    };

}

}

#endif //COPYSTATEMACHINE_STATEMACHINELOGINITIALIZER_H
