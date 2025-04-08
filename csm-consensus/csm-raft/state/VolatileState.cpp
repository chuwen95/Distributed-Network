//
// Created by chu on 3/18/25.
//

#include "VolatileState.h"

using namespace csm::consensus;

std::uint64_t VolatileState::commitIndex() const
{
    return m_commitIndex;
}

std::uint64_t VolatileState::lastAppliedIndex() const
{
    return m_lastAppliedIndex;
}
