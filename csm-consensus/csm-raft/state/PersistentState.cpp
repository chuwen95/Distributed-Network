//
// Created by chu on 3/18/25.
//

#include "PersistentState.h"

#include "csm-utilities/Logger.h"

using namespace csm::consensus;

PersistentState::PersistentState(RaftMetadataStorage::Ptr raftMetadataStorage) : m_raftMetadataStorage(std::move(raftMetadataStorage))
{
}

int PersistentState::init()
{
    std::uint64_t currentTerm;
    if (-1 == m_raftMetadataStorage->getCurrentTerm(currentTerm))
    {
        LOG->write(utilities::LogType::Log_Error, FILE_INFO, "init persistent state failed, get current term from database failed");
        return -1;
    }

    LOG->write(utilities::LogType::Log_Info, FILE_INFO, "init persistent state successfully, get current term from database: ", currentTerm);
    m_currentTerm = currentTerm;

    return 0;
}

int PersistentState::currentTerm(std::uint64_t& term) const
{
    return m_currentTerm;
}

int PersistentState::setCurrentTerm(std::uint64_t term)
{
    if (0 != m_raftMetadataStorage->setCurrentTerm(term))
    {
        LOG->write(utilities::LogType::Log_Error, FILE_INFO, "set current term from database failed");
        return -1;
    }

    LOG->write(utilities::LogType::Log_Debug, FILE_INFO, "set current term to database successfully, term: ", term);
    m_currentTerm = term;

    return 0;
}

int PersistentState::increaseTerm()
{
    if (-1 == setCurrentTerm(m_currentTerm + 1))
    {
        LOG->write(utilities::LogType::Log_Error, FILE_INFO, "increase term failed");
        return -1;
    }

    ++m_currentTerm;
    LOG->write(utilities::LogType::Log_Debug, FILE_INFO, "increase term succesfully, current term: ", m_currentTerm.load());

    return 0;
}

csm::NodeId PersistentState::voteFor() const
{
    return m_voteFor;
}

void PersistentState::setVoteFor(const NodeId& voteFor)
{
    m_voteFor = voteFor;
}

void PersistentState::addEntry(stmclog::Entry::Ptr entry)
{
    m_logs.emplace(std::move(entry));
}
