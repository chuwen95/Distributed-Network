//
// Created by ChuWen on 10/11/23.
//

#include "NodeConfig.h"
#include "csm-utilities/StringTool.h"

using namespace csm::tool;

constexpr std::uint32_t c_maxClusterServerNum{ 9999 };

int NodeConfig::init(const std::string &configFile)
{
    m_configFile = configFile;

    inipp::Ini<char> ini;
    std::ifstream file(m_configFile);
    if(false == file.is_open())
    {
        std::cout << "open config file failed" << std::endl;
        return -1;
    }

    std::filesystem::path absolutePath = std::filesystem::absolute(m_configFile);
    m_configDir = absolutePath.parent_path();

    ini.parse(file);

    parseClusterConfig(ini);
    parseRpcConfig(ini);
    parseNetworkConfig(ini);
    parseLogConfig(ini);

    file.close();

    return 0;
}

std::string NodeConfig::configFile()
{
    return m_configFile;
}

std::string NodeConfig::id()
{
    return m_id;
}

bool NodeConfig::startAsClient()
{
    return m_startAsClient;
}

std::string NodeConfig::rpcIp()
{
    return m_rpcIp;
}

unsigned short NodeConfig::rpcPort()
{
    return m_rpcPort;
}

std::string NodeConfig::p2pIp()
{
    return m_p2pIp;
}

unsigned short NodeConfig::p2pPort()
{
    return m_p2pPort;
}

std::string NodeConfig::nodesFile()
{
    return m_nodesFile;
}

std::size_t NodeConfig::slaveReactorNum()
{
    return m_slaveReactorNum;
}

std::size_t NodeConfig::redispatchInterval()
{
    return m_redispatchInterval;
}

std::size_t NodeConfig::sessionDataWorkerNum()
{
    return m_sessionDataWorkerNum;
}

bool NodeConfig::enableFileLog()
{
    return m_enableFileLog;
}

bool NodeConfig::consoleOutput()
{
    return m_consoleOutput;
}

csm::utilities::LogType NodeConfig::logType()
{
    return m_logType;
}

std::string NodeConfig::logPath()
{
    return m_logPath;
}

int NodeConfig::loadLogConfig()
{
    std::cout << "load log config from: " << m_configFile << std::endl;

    inipp::Ini<char> ini;
    std::ifstream file(m_configFile);
    if(false == file.is_open())
    {
        std::cout << "open config file failed" << std::endl;
        return -1;
    }
    ini.parse(file);
    parseLogConfig(ini);
    file.close();

    return 0;
}

template<typename T>
T getValue(inipp::Ini<char>& ini, const std::string&sectionName, const std::string& key, const T& defaultValue)
{
    auto sectionIter = ini.sections.find(sectionName);
    if(ini.sections.end() == sectionIter)
    {
        return defaultValue;
    }

    auto iter = sectionIter->second.find(key);
    if(sectionIter->second.end() == iter)
    {
        return defaultValue;
    }
    else
    {
        return csm::utilities::convertFromString<T>(iter->second);
    }
}

int NodeConfig::parseClusterConfig(inipp::Ini<char> &ini)
{
    m_id = getValue(ini, "cluster", "id", m_id);
    m_clusterServerIds.emplace_back(m_id);

    const std::string emptyServerId{ "__server_id_not_exist__" };
    for(std::uint32_t i = 0; i < c_maxClusterServerNum; ++i)
    {
        std::string key = "serverId_" + utilities::convertToString(i);
        std::string serverId = getValue(ini, "cluster", "serverId", emptyServerId);
        if(emptyServerId == serverId)
        {
            break;
        }
        m_clusterServerIds.emplace_back(serverId);
    }

    return 0;
}

int NodeConfig::parseRpcConfig(inipp::Ini<char>& ini)
{
    m_rpcIp = getValue(ini, "rpc", "ip", m_rpcIp);
    m_rpcPort = getValue(ini, "rpc", "port", m_rpcPort);

    return 0;
}

int NodeConfig::parseNetworkConfig(inipp::Ini<char>& ini)
{
    m_p2pIp = getValue(ini, "network", "ip", m_p2pIp);
    m_p2pPort = getValue(ini, "network", "port", m_p2pPort);
    m_nodesFile = getValue(ini, "network", "nodes_file", m_nodesFile);
    m_sessionDataWorkerNum = getValue(ini, "network", "packet_process_worker_num", m_sessionDataWorkerNum);

    m_slaveReactorNum = getValue(ini, "reactor", "slave_reactor", m_slaveReactorNum);
    m_redispatchInterval = getValue(ini, "reactor", "redispatch_interval", m_redispatchInterval);


    m_startAsClient = getValue(ini, "network", "start_as_client", m_startAsClient);

    if('/' != m_nodesFile[0])
    {
        if('/' != m_nodesFile.back())
        {
            m_nodesFile = m_configDir + "/" + m_nodesFile;
        }
    }

    return 0;
}

int NodeConfig::parseLogConfig(inipp::Ini<char>& ini)
{
    m_enableFileLog = getValue(ini, "logger", "enable_file_log", m_enableFileLog);
    m_consoleOutput = getValue(ini, "logger", "console_output", m_consoleOutput);
    std::string logLevel = getValue(ini, "logger", "level", std::string("info"));
    if("trace" == logLevel)
    {
        m_logType = csm::utilities::LogType::Log_Trace;
    }
    else if("debug" == logLevel)
    {
        m_logType = csm::utilities::LogType::Log_Debug;
    }
    else if("info" == logLevel)
    {
        m_logType = csm::utilities::LogType::Log_Info;
    }
    else if("warning" == logLevel)
    {
        m_logType = csm::utilities::LogType::Log_Warning;
    }
    else if("error" == logLevel)
    {
        m_logType = csm::utilities::LogType::Log_Error;
    }
    else
    {
        std::cout << "log type parse error: must be trace/debug/info/warning/error" << std::endl;
        return -1;
    }
    m_logPath = getValue(ini, "logger", "path", m_logPath);

    if(true == m_logPath.empty())
    {
        m_logPath = "./logs";
    }

    // 如果是相对路径，则将配置文件所在路径作为基准路径
    if('/' != m_logPath[0])
    {
        if('/' != m_logPath.back())
        {
            m_logPath = m_configDir + "/" + m_logPath;
        }
    }

    return 0;
}

const std::vector<std::string> &NodeConfig::clusterServerIds() const
{
    return m_clusterServerIds;
}
