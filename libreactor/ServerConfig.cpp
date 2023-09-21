//
// Created by root on 9/20/23.
//

#include "ServerConfig.h"
#include "libcomponents/StringTool.h"

namespace server
{

    int ServerConfig::init(const std::string &configFile)
    {
        inipp::Ini<char> ini;
        std::ifstream file(configFile);
        if(false == file.is_open())
        {
            std::cout << "open config file failed" << std::endl;
            return -1;
        }
        ini.parse(file);

        parseNetworkConfig(ini);
        parseReactorConfig(ini);
        parsePacketProcessConfig(ini);
        parseLogConfig(ini);

        return 0;
    }

    int ServerConfig::uninit()
    {
        return 0;
    }

    std::string ServerConfig::ip()
    {
        return m_ip;
    }

    unsigned short ServerConfig::port()
    {
        return m_port;
    }

    std::size_t ServerConfig::slaveReactorNum()
    {
        return m_slaveReactorNum;
    }

    std::size_t ServerConfig::redispatchInterval()
    {
        return m_redispatchInterval;
    }

    std::size_t ServerConfig::packetProcessThreadNum()
    {
        return m_packetProcessThreadNum;
    }

    bool ServerConfig::enableFileLog()
    {
        return m_enableFileLog;
    }

    bool ServerConfig::consoleOutput()
    {
        return m_consoleOutput;
    }

    components::LogType ServerConfig::logType()
    {
        return m_logType;
    }

    std::string ServerConfig::logPath()
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

    int ServerConfig::parseNetworkConfig(inipp::Ini<char>& ini)
    {
        m_ip = getValue(ini, "network", "ip", m_ip);
        m_port = getValue(ini, "network", "port", m_port);

        return 0;
    }

    int ServerConfig::parseReactorConfig(inipp::Ini<char>& ini)
    {
        m_slaveReactorNum = getValue(ini, "reactor", "slave_reactor", m_slaveReactorNum);
        m_redispatchInterval = getValue(ini, "reactor", "redispatch_interval", m_redispatchInterval);

        return 0;
    }

    int ServerConfig::parsePacketProcessConfig(inipp::Ini<char>& ini)
    {
        m_packetProcessThreadNum = getValue(ini, "packet_process", "thread_pool_thread_num", m_packetProcessThreadNum);

        return 0;
    }

    int ServerConfig::parseLogConfig(inipp::Ini<char>& ini)
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