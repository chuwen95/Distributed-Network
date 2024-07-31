//
// Created by ChuWen on 2024/6/22.
//

#include "StateMachineLog.h"

using namespace csm::stmclog;

StateMachineLog::StateMachineLog(StateMachineLogConfig::Ptr stateMachineLogConfig) :
        m_stateMachineLogConfig(std::move(stateMachineLogConfig))
{}

int StateMachineLog::init()
{
    return 0;
}

int StateMachineLog::start()
{
    return 0;
}

int StateMachineLog::stop()
{
    return 0;
}
