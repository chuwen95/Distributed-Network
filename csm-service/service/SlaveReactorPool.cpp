//
// Created by chu on 11/14/25.
//

#include "SlaveReactorPool.h"

using namespace csm::service;

SlaveReactorPool::SlaveReactorPool(std::size_t size)
{
    for (int i = 0; i < size; ++i)
    {
        m_slaveReactors.emplace_back(std::make_unique<SlaveReactor>(i));
    }
}

SlaveReactor* SlaveReactorPool::slaveReactor(std::size_t index)
{
    if (index >= m_slaveReactors.size())
    {
        return nullptr;
    }

    return m_slaveReactors[index].get();
}

std::vector<SlaveReactor*> SlaveReactorPool::slaveReactors()
{
    std::vector<SlaveReactor*> slaveReactors;
    for (auto& slaveReactor : m_slaveReactors)
    {
        slaveReactors.emplace_back(slaveReactor.get());
    }

    return slaveReactors;
}