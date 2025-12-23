//
// Created by ChuWen on 2024/6/22.
//

#include "StateMachineLog.h"

#include "json/json.h"
#include "csm-utilities/StringTool.h"
#include "csm-tool/ClusterConfigurationSerializer.h"
#include "protocol/packet/Entry.h"

using namespace csm::stmclog;

constexpr const char* const c_stmclogTableName{ "__stmc_log__" };

constexpr const char* const c_stmcLogBeginIndexKey{ "begin_index" };
constexpr const char* const c_stmcLogEndIndexKey{ "end_index" };

StateMachineLog::StateMachineLog(StateMachineLogConfig::Ptr stateMachineLogConfig) :
        m_stateMachineLogConfig(std::move(stateMachineLogConfig))
{}

int StateMachineLog::init()
{
    m_endIndex = endIndex();
    if (0 != m_endIndex)
    {
        std::string value;
        m_stateMachineLogConfig->storage()->get(c_stmclogTableName, "1", value);

        Entry entry;
        entry.decode(std::vector<char>(value.begin(), value.end()));
        if (EntryType::Configuration != entry.entryType())
        {
            LOG->write(utilities::LogType::Log_Debug, FILE_INFO, "state machine log error, first log entry type is not configuration");
            return -1;
        }
    }

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
    std::string jsonString = tool::ClusterConfigurationSerializer::serialize(serverIds);

    // 构造日志Entry
    Entry entry;
    entry.setIndex(0);
    entry.setEntryType(EntryType::Configuration);
    entry.setTerm(0);
    entry.setData(jsonString);

    // 编码Entry
    std::vector<char> data = entry.encode();

    // 将类型写入到数据前面，占据1个字节
    char type = static_cast<char>(EntryType::Configuration);
    data.insert(data.begin(), type);

    // 写入Entry Data
    if(-1 == m_stateMachineLogConfig->storage()->set(c_stmclogTableName, "1", data))
    {
        LOG->write(utilities::LogType::Log_Error, FILE_INFO, "write log index 1 failed");
        return -1;
    }
    LOG->write(utilities::LogType::Log_Debug, FILE_INFO, "write log index 1 successfully");

    return increaseEndIndex();
}

EntryType StateMachineLog::getLogType(const std::uint64_t index) const
{
    // 获取Entry Data
    std::string data;
    if(-1 == m_stateMachineLogConfig->storage()->get(c_stmclogTableName, std::to_string(index), data))
    {
        LOG->write(utilities::LogType::Log_Error, FILE_INFO, "get log index 1 failed");
        return EntryType::Unknown;
    }

    char type = data[0];
    return static_cast<EntryType>(type);
}

Entry::Ptr StateMachineLog::getLogEntry(const std::uint64_t index) const
{
    // 获取Entry Data
    std::string data;
    if(-1 == m_stateMachineLogConfig->storage()->get(c_stmclogTableName, std::to_string(index), data))
    {
        LOG->write(utilities::LogType::Log_Error, FILE_INFO, "get log index 1 failed");
        return nullptr;
    }

    Entry::Ptr entry = std::make_shared<Entry>();
    entry->decode(std::vector<char>(data.begin() + 1, data.end()));

    return entry;
}
