//
// Created by root on 10/11/23.
//

#include "NodeConfig.h"
#include "libcomponents/StringTool.h"

namespace tool
{

    int NodeConfig::init(const std::string &configFile)
    {
        inipp::Ini<char> ini;
        std::ifstream file(configFile);
        if(false == file.is_open())
        {
            std::cout << "open config file failed" << std::endl;
            return -1;
        }
        ini.parse(file);

        parseInfoConfig(ini);
        parseFeatureConfig(ini);
        parseNetworkConfig(ini);
        parseReactorConfig(ini);
        parsePacketProcessConfig(ini);
        parseLogConfig(ini);

        return 0;
    }

    int NodeConfig::uninit()
    {
        return 0;
    }

    std::string NodeConfig::id()
    {
        return m_id;
    }

    bool NodeConfig::startAsClient()
    {
        return m_startAsClient;
    }

    std::string NodeConfig::ip()
    {
        return m_ip;
    }

    unsigned short NodeConfig::port()
    {
        return m_port;
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

    std::size_t NodeConfig::packetProcessThreadNum()
    {
        return m_packetProcessThreadNum;
    }

    bool NodeConfig::enableFileLog()
    {
        return m_enableFileLog;
    }

    bool NodeConfig::consoleOutput()
    {
        return m_consoleOutput;
    }

    components::LogType NodeConfig::logType()
    {
        return m_logType;
    }

    std::string NodeConfig::logPath()
    {
        return m_logPath;
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
            return components::string_tools::convertFromString<T>(iter->second);
        }
    }

    int NodeConfig::parseInfoConfig(inipp::Ini<char> &ini)
    {
        m_id = getValue(ini, "info", "id", m_id);

        return 0;
    }

    int NodeConfig::parseFeatureConfig(inipp::Ini<char> &ini)
    {
        m_startAsClient = getValue(ini, "feature", "start_as_client", m_startAsClient);

        return 0;
    }

    int NodeConfig::parseNetworkConfig(inipp::Ini<char>& ini)
    {
        m_ip = getValue(ini, "network", "ip", m_ip);
        m_port = getValue(ini, "network", "port", m_port);
        m_nodesFile = getValue(ini, "network", "nodes_file", m_nodesFile);

        return 0;
    }

    int NodeConfig::parseReactorConfig(inipp::Ini<char>& ini)
    {
        m_slaveReactorNum = getValue(ini, "reactor", "slave_reactor", m_slaveReactorNum);
        m_redispatchInterval = getValue(ini, "reactor", "redispatch_interval", m_redispatchInterval);

        return 0;
    }

    int NodeConfig::parsePacketProcessConfig(inipp::Ini<char>& ini)
    {
        m_packetProcessThreadNum = getValue(ini, "packet_process", "thread_pool_thread_num", m_packetProcessThreadNum);

        return 0;
    }

    int NodeConfig::parseLogConfig(inipp::Ini<char>& ini)
    {
        m_enableFileLog = getValue(ini, "logger", "enable_file_log", m_enableFileLog);
        m_consoleOutput = getValue(ini, "logger", "console_output", m_consoleOutput);
        std::string logLevel = getValue(ini, "logger", "level", std::string("info"));
        if("trace" == logLevel)
        {
            m_logType = components::LogType::Log_Trace;
        }
        else if("debug" == logLevel)
        {
            m_logType = components::LogType::Log_Debug;
        }
        else if("info" == logLevel)
        {
            m_logType = components::LogType::Log_Info;
        }
        else if("warning" == logLevel)
        {
            m_logType = components::LogType::Log_Warning;
        }
        else if("error" == logLevel)
        {
            m_logType = components::LogType::Log_Error;
        }
        else
        {
            std::cout << "log type parse error: must be trace/debug/info/warning/error" << std::endl;
            return -1;
        }
        m_logPath = getValue(ini, "log", "path", m_logPath);

        return 0;
    }

}