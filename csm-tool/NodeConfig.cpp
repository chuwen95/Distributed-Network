//
// Created by ChuWen on 10/11/23.
//

#include "NodeConfig.h"

#include <json/value.h>
#include <json/reader.h>

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
        std::cerr << "open config file failed, file: " << m_configFile << std::endl;
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

std::string NodeConfig::configFile() const
{
    return m_configFile;
}

csm::NodeId NodeConfig::id() const
{
    return m_id;
}

const csm::NodeIds& NodeConfig::clusterServerIds() const
{
    return m_clusterServerIds;
}

std::uint32_t NodeConfig::minRandomVoteTimeout() const
{
    return m_minRandomVoteTimeout;
}

std::uint32_t NodeConfig::maxRandomVoteTimeout() const
{
    return m_maxRandomVoteTimeout;
}

std::string NodeConfig::httpRpcIp() const
{
    return m_httpRpcIp;
}

unsigned short NodeConfig::httpRpcPort() const
{
    return m_httpRpcPort;
}

std::string NodeConfig::tcpRpcIp() const
{
    return m_tcpRpcIp;
}

unsigned short NodeConfig::tcpRpcPort() const
{
    return m_tcpRpcPort;
}

std::string NodeConfig::p2pIp() const
{
    return m_p2pIp;
}

unsigned short NodeConfig::p2pPort() const
{
    return m_p2pPort;
}

std::string NodeConfig::nodesFile() const
{
    return m_nodesFile;
}

std::size_t NodeConfig::slaveReactorNum() const
{
    return m_slaveReactorNum;
}

std::size_t NodeConfig::redispatchInterval() const
{
    return m_redispatchInterval;
}

std::size_t NodeConfig::sessionDataDecoderWorkerNum() const
{
    return m_sessionDataDecodeWorkerNum;
}

std::size_t NodeConfig::sessionDataProcessWorkerNum() const
{
    return m_sessionDataProcessWorkerNum;
}

bool NodeConfig::enableFileLog() const
{
    return m_enableFileLog;
}

bool NodeConfig::consoleOutput() const
{
    return m_consoleOutput;
}

csm::utilities::LogType NodeConfig::logType() const
{
    return m_logType;
}

std::string NodeConfig::logPath() const
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
        std::cerr << "open config file failed, file: " << m_configFile << std::endl;
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
    // 本节点ID
    m_id = getValue(ini, "cluster", "id", m_id);

    // 集群中所有服务器ID
    std::string servers = getValue(ini, "cluster", "servers", std::string());
    Json::Value root;
    if (false == Json::Reader().parse(servers, root))
    {
        std::cerr << "parse config file failed, servers: " << servers << std::endl;
        return -1;
    }
    for (auto iter = root.begin(); iter != root.end(); ++iter)
    {
        if (Json::ValueType::stringValue != iter->type())
        {
            std::cerr << "parse config file failed, servers are not composed of string: " << servers << std::endl;
            return -1;
        }

        m_clusterServerIds.emplace_back(iter->asString());
    }

    return 0;
}

int NodeConfig::parseRpcConfig(inipp::Ini<char>& ini)
{
    m_httpRpcIp = getValue(ini, "rpc", "http_listen_ip", m_httpRpcIp);
    m_httpRpcPort = getValue(ini, "rpc", "http_listen_port", m_httpRpcPort);

    m_tcpRpcIp = getValue(ini, "rpc", "tcp_listen_ip", m_httpRpcIp);
    m_tcpRpcPort = getValue(ini, "rpc", "tcp_listen_port", m_httpRpcPort);

    return 0;
}

int NodeConfig::parseNetworkConfig(inipp::Ini<char>& ini)
{
    m_p2pIp = getValue(ini, "network", "ip", m_p2pIp);
    m_p2pPort = getValue(ini, "network", "port", m_p2pPort);
    m_nodesFile = getValue(ini, "network", "nodes_file", m_nodesFile);
    m_sessionDataDecodeWorkerNum = getValue(ini, "network", "packet_decode_worker_num", m_sessionDataDecodeWorkerNum);
    m_sessionDataProcessWorkerNum = getValue(ini, "network", "packet_process_worker_num", m_sessionDataProcessWorkerNum);

    m_slaveReactorNum = getValue(ini, "reactor", "slave_reactor", m_slaveReactorNum);
    m_redispatchInterval = getValue(ini, "reactor", "redispatch_interval", m_redispatchInterval);

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
