//
// Created by ChuWen on 2024/7/31.
//

#include "StateMachineLogFactory.h"
#include "StateMachineLogConfig.h"

using namespace csm::stmclog;

StateMachineLogFactory::StateMachineLogFactory(csm::tool::NodeConfig::Ptr nodeConfig, csm::storage::Storage::Ptr storage) :
    m_nodeConfig(nodeConfig), m_storage(storage)
{
}

StateMachineLog::Ptr StateMachineLogFactory::createStateMachineLog()
{
    StateMachineLogConfig::Ptr stateMachineLogConfig = std::make_shared<StateMachineLogConfig>(m_nodeConfig, m_storage);

    return std::make_shared<StateMachineLog>(stateMachineLogConfig);
}

