//
// Created by chu on 3/30/25.
//

#include "RaftMetadataStorage.h"

#include "csm-utilities/Logger.h"

using namespace csm::consensus;

constexpr const char* const c_raftMetadataTableName{ "__stmc_log__" };

constexpr const char* const c_currentTermKey{ "current_term" };

RaftMetadataStorage::RaftMetadataStorage(storage::Storage::Ptr storage) : m_storage(std::move(storage))
{
}

int RaftMetadataStorage::getCurrentTerm(std::uint64_t& term) const
{
    std::string currentTermValue;
    if(-1 == m_storage->get(c_raftMetadataTableName, c_currentTermKey, currentTermValue))
    {
        LOG->write(utilities::LogType::Log_Error, FILE_INFO, "get current term from database failed");
        return -1;
    }
    LOG->write(utilities::LogType::Log_Debug, FILE_INFO, "get current term from database successfully: ", currentTermValue);

    term = csm::utilities::convertFromString<std::uint64_t>(currentTermValue);

    return 0;
}

int RaftMetadataStorage::setCurrentTerm(std::uint64_t term)
{
    std::string endIndexValue = utilities::convertToString(term);

    if(-1 == m_storage->set(c_raftMetadataTableName, c_currentTermKey, std::vector<char>(endIndexValue.begin(), endIndexValue.end())))
    {
        LOG->write(utilities::LogType::Log_Error, FILE_INFO, "write current term to database failed");
        return -1;
    }
    LOG->write(utilities::LogType::Log_Error, FILE_INFO, "write current term to database successfully");

    return 0;
}
