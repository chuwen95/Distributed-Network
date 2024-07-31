//
// Created by ChuWen on 2024/7/20.
//

#include "StateMachineLogConfig.h"

using namespace csm::stmclog;

StateMachineLogConfig::StateMachineLogConfig(storage::Storage::Ptr storage) :
    m_storage(std::move(storage))
{}

csm::storage::Storage::Ptr StateMachineLogConfig::storage()
{
    return m_storage;
}
