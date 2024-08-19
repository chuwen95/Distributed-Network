//
// Created by ChuWen on 2024/6/22.
//

#include "StateMachineLog.h"
#include "csm-utilities/StringTool.h"
#include "protocol/packet/Entry.h"
#include "json/json.h"

using namespace csm::stmclog;

constexpr const char* const c_stmclogTableName{ "__stmc_log__" };
constexpr const char* const c_stmcLogBeginIndexKey{ "begin_index" };
constexpr const char* const c_stmcLogEndIndexKey{ "end_index" };

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

std::uint64_t StateMachineLog::beginIndex()
{
    std::string beginIndexValue;
    if(-1 == m_stateMachineLogConfig->storage()->get(c_stmclogTableName, c_stmcLogBeginIndexKey, beginIndexValue))
    {
        LOG->write(utilities::LogType::Log_Error, FILE_INFO, "get begin index failed");
        return 0;
    }
    LOG->write(utilities::LogType::Log_Debug, FILE_INFO, "get begin index successfully: ", beginIndexValue);

    return csm::utilities::convertFromString<std::uint64_t>(beginIndexValue);
}

std::uint64_t StateMachineLog::endIndex()
{
    if(0 != m_endIndex)
    {
        return m_endIndex;
    }

    std::string endIndexValue;
    if(-1 == m_stateMachineLogConfig->storage()->get(c_stmclogTableName, c_stmcLogEndIndexKey, endIndexValue))
    {
        LOG->write(utilities::LogType::Log_Error, FILE_INFO, "get end index failed");
        return 0;
    }
    LOG->write(utilities::LogType::Log_Debug, FILE_INFO, "get end index successfully: ", endIndexValue);

    m_endIndex = csm::utilities::convertFromString<std::uint64_t>(endIndexValue);

    return m_endIndex;
}

int StateMachineLog::increaseEndIndex()
{
    std::string endIndexValue = utilities::convertToString(m_endIndex + 1);
    if(-1 == m_stateMachineLogConfig->storage()->set(c_stmclogTableName, c_stmcLogEndIndexKey, std::vector<char>(endIndexValue.begin(), endIndexValue.end())))
    {
        LOG->write(utilities::LogType::Log_Error, FILE_INFO, "increate end index in database failed");
        return -1;
    }
    LOG->write(utilities::LogType::Log_Error, FILE_INFO, "increate end index in database successfully");
    ++m_endIndex;

    return 0;
}

int StateMachineLog::buildInitialConfigurationLog()
{
    // 构造集群配置日志
    std::vector<std::string> serverIds = m_stateMachineLogConfig->nodeConfig()->clusterServerIds();

    Json::Value root;
    for(const std::string& serverId : serverIds)
    {
        root["clusterServerIds"].append(serverId);
    }

    Json::FastWriter fastWriter;
    std::string jsonString = fastWriter.write(root);

    // 构造日志Entry
    Entry entry;
    entry.setIndex(1);
    entry.setEntryType(EntryType::Configuration);
    entry.setTerm(0);
    entry.setData(jsonString);

    // 编码Entry
    std::vector<char> data = entry.encode();
    // 写入Entry Data
    if(-1 == m_stateMachineLogConfig->storage()->set(c_stmclogTableName, "1", data))
    {
        LOG->write(utilities::LogType::Log_Error, FILE_INFO, "write log index 1 failed");
        return -1;
    }
    LOG->write(utilities::LogType::Log_Debug, FILE_INFO, "write log index 1 successfully");

    return increaseEndIndex();
}

const std::vector<std::string> &StateMachineLog::clusterServerIds()
{
    return m_stateMachineLogConfig->nodeConfig()->clusterServerIds();
}
