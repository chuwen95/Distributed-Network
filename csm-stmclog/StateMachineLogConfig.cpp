//
// Created by ChuWen on 2024/7/20.
//

#include "StateMachineLogConfig.h"

using namespace csm::stmclog;

StateMachineLogConfig::StateMachineLogConfig(tool::NodeConfig::Ptr nodeConfig, storage::Storage::Ptr storage) :
    m_nodeConfig(std::move(nodeConfig)), m_storage(std::move(storage))
{}

csm::tool::NodeConfig::Ptr StateMachineLogConfig::nodeConfig()
{
    return m_nodeConfig;
}

csm::storage::Storage::Ptr StateMachineLogConfig::storage()
{
    return m_storage;
}
