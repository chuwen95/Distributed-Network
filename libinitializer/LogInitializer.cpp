//
// Created by ChuWen on 2024/2/25.
//

#include "LogInitializer.h"

struct LogLevelResetHandler
{
    static tools::NodeConfig::Ptr nodeConfig;

    static void handle(int sig)
    {
        nodeConfig->loadLogConfig();
        components::Singleton<components::Logger>::instance()->setLogLevel(nodeConfig->logType());
        components::Singleton<components::Logger>::instance()->setConsoleOutput(nodeConfig->consoleOutput());

        components::Singleton<components::Logger>::instance()->write(components::LogType::Log_Info, FILE_INFO,
                                                                     "reload log config successfully, log level: ", static_cast<int>(nodeConfig->logType()));
    }
};
tools::NodeConfig::Ptr LogLevelResetHandler::nodeConfig = nullptr;

namespace initializer
{

    LogInitializer::LogInitializer(tools::NodeConfig::Ptr nodeConfig) : m_nodeConfig(std::move(nodeConfig))
    {}

    int LogInitializer::init()
    {
        // 设置Logger
        if(-1 == components::Singleton<components::Logger>::instance()->init(m_nodeConfig->enableFileLog(), m_nodeConfig->logPath()))
        {
            return -1;
        }
        components::Singleton<components::Logger>::instance()->setLogLevel(m_nodeConfig->logType());
        components::Singleton<components::Logger>::instance()->setConsoleOutput(m_nodeConfig->consoleOutput());

        LogLevelResetHandler::nodeConfig = m_nodeConfig;
        signal(SIGUSR2, LogLevelResetHandler::handle);

        components::Singleton<components::Logger>::instance()->write(components::LogType::Log_Info, FILE_INFO,
                                                                     "register SIGUSR2 for reload log config");

        return 0;
    }

    int LogInitializer::uninit()
    {
        return components::Singleton<components::Logger>::instance()->uninit();
    }

    int LogInitializer::start()
    {
        return components::Singleton<components::Logger>::instance()->start();
    }

    int LogInitializer::stop()
    {
        return components::Singleton<components::Logger>::instance()->stop();
    }

} // initializer