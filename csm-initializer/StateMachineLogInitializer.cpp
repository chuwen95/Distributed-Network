//
// Created by ChuWen on 2024/7/31.
//

#include "StateMachineLogInitializer.h"
#include "csm-stmclog/StateMachineLogFactory.h"

using namespace csm::initializer;

StateMachineLogInitializer::StateMachineLogInitializer(csm::storage::Storage::Ptr storage)
{
    stmclog::StateMachineLogFactory::Ptr stateMachineLogFactory = std::make_shared<stmclog::StateMachineLogFactory>(storage);
    m_stateMachineLog = stateMachineLogFactory->createStateMachineLog();
}

int StateMachineLogInitializer::init()
{
    return m_stateMachineLog->init();
}

int StateMachineLogInitializer::start()
{
    return m_stateMachineLog->start();
}

int StateMachineLogInitializer::stop()
{
    return m_stateMachineLog->stop();
}
