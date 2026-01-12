//
// Created by ChuWen on 2024/2/25.
//

#include "AppLogInitializer.h"

#ifdef __linux__
#include <signal.h>
#endif

struct LogLevelResetHandler
{
    static csm::tool::NodeConfig* nodeConfig;

    static void handle(int sig)
    {
        nodeConfig->loadLogConfig();
        LOG->setLogLevel(nodeConfig->logType());
        LOG->setConsoleOutput(nodeConfig->consoleOutput());

        LOG->write(csm::utilities::LogType::Log_Info, FILE_INFO,
                                                                     "reload log config successfully, log level: ", static_cast<int>(nodeConfig->logType()));
    }
};
csm::tool::NodeConfig* LogLevelResetHandler::nodeConfig = nullptr;

using namespace csm::initializer;

AppLogInitializer::AppLogInitializer(tool::NodeConfig* nodeConfig) : m_nodeConfig(nodeConfig)
{}

int AppLogInitializer::init()
{
    // 设置Logger
    LOG->setEnableFileLog(m_nodeConfig->enableFileLog());
    LOG->setLogDirPath(m_nodeConfig->logPath());
    if (-1 == LOG->init())
    {
        return -1;
    }
    LOG->setLogLevel(m_nodeConfig->logType());
    LOG->setConsoleOutput(m_nodeConfig->consoleOutput());

    LogLevelResetHandler::nodeConfig = m_nodeConfig;
    signal(SIGUSR2, LogLevelResetHandler::handle);

    LOG->write(utilities::LogType::Log_Info, FILE_INFO, "register SIGUSR2 for reload log config");

    return 0;
}

int AppLogInitializer::start()
{
    return LOG->start();
}

int AppLogInitializer::stop()
{
    return LOG->stop();
}