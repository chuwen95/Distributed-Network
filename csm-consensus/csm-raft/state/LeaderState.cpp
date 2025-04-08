//
// Created by chu on 3/18/25.
//

#include "LeaderState.h"

using namespace csm::consensus;

std::vector<std::uint64_t> LeaderState::logIndexNextSend() const
{
    return m_logIndexNextSend;
}

std::vector<std::uint64_t> LeaderState::logIndexMatch() const
{
    return m_logIndexMatch;
}
