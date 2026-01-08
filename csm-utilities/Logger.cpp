//
// Created by ChuWen on 9/6/23.
//

#include "Logger.h"

using namespace csm::utilities;

#if 1

void Logger::setEnableFileLog(bool enableFileLog)
{
    m_enableFileLog = enableFileLog;
}

void Logger::setLogDirPath(const std::string& logDirPath)
{
    m_logDirPath = logDirPath;
}

void Logger::setLogBufferSize(std::size_t bufferSize)
{
    m_bufferSize = bufferSize;
}

int Logger::init()
{
    if (true == m_enableFileLog)
    {
        std::filesystem::create_directory(m_logDirPath);

        std::string logFilename = generateLogFilename();
        std::string logFullFilename = generateLogFullFilename(m_logDirPath, logFilename);

        m_file.open(logFullFilename, std::ios::out);
        if (false == m_file.is_open())
        {
            std::cerr << "open log file failed: " << logFullFilename << ", error state: "
                << m_file.rdstate() << ", errno: " << errno << ", " << strerror(errno)<< std::endl;
            return -1;
        }

        m_buffer.resize(m_bufferSize);

        const auto expression = [this](const std::stop_token& st) {
            if (true == st.stop_requested())
            {
                return;
            }

            std::vector<char> buffer;
            {
                std::unique_lock<std::mutex> ulock(x_buffer);
                m_bufferCv.wait(ulock, [this, st]() { return false == m_buffer.empty() || true == st.stop_requested(); });

                m_buffer.swap(buffer);
            }

            m_file.write(buffer.data(), buffer.size());
            m_file.flush();
        };
        m_thread = std::make_unique<Thread>(expression, 1, "logger");
        m_thread->start();
    }

    return 0;
}

int Logger::start()
{
    if (true == m_enableFileLog)
    {
        m_thread->start();
    }

    return 0;
}

int Logger::stop()
{
    if (true == m_enableFileLog)
    {
        m_thread->stop();
        m_bufferCv.notify_one();
    }

    return 0;
}

void Logger::setLogLevel(const utilities::LogType level)
{
    m_logLevel = level;
}

void Logger::setConsoleOutput(bool enable)
{
    m_consoleOutput = enable;
}

std::string Logger::generateLogFilename()
{
    // 获取当前时间
    std::time_t currentTime = std::time(nullptr);
    std::tm* localTime = std::localtime(&currentTime);

    // 格式化时间为所需的文件名格式
    std::stringstream filenameStream;
    filenameStream << "log_" << std::setfill('0') << std::setw(4) << (localTime->tm_year + 1900)
                   << std::setw(2) << (localTime->tm_mon + 1) << std::setw(2) << localTime->tm_mday
                   << std::setw(2) << localTime->tm_hour << '.' << std::setw(2) << localTime->tm_min
                   << ".log";

    std::string logFilename = filenameStream.str();

    return logFilename;
}

std::string Logger::generateLogFullFilename(const std::string& logDirPath, const std::string& logFilename)
{
    std::string logFullFilename;

    if(true == m_logDirPath.empty())
    {
        logFullFilename = "./" + logFilename;
    }
    else
    {
        if(m_logDirPath.back() == '/')
        {
            logFullFilename = m_logDirPath + logFilename;
        }
        else
        {
            logFullFilename = m_logDirPath + "/" + logFilename;
        }
    }

    return logFullFilename;
}

#else

Logger::Logger(const bool enableFileLog, const std::string &path) :
    m_enableFileLog(enableFileLog), m_path(path)
{ }

int Logger::init()
{
    if (true == m_enableFileLog)
    {
        std::filesystem::create_directory(m_path);

        std::string logFilename = generateLogFilename();

        std::string logFullFilename;
        if(true == m_path.empty())
        {
            logFullFilename = "./" + logFilename;
        }
        else
        {
            if(m_path.back() == '/')
            {
                logFullFilename = m_path + logFilename;
            }
            else
            {
                logFullFilename = m_path + "/" + logFilename;
            }
        }

        m_file.open(logFullFilename, std::ios::out);
        if (false == m_file.is_open())
        {
            std::cerr << "open log file failed: " << logFullFilename << ", error state: " << m_file.rdstate() << ", errno: " << errno << ", " << strerror(errno)<< std::endl;
            return -1;
        }
    }

    const auto logWritter = [this]() {
        std::string buffer;

        std::string log;

        while(true == m_logs.try_dequeue(log))
        {
            buffer.append(log);
        }
        m_file << buffer;
    };
    m_thread.init(logWritter, 10, "logger");

    return 0;
}

int Logger::start()
{
    m_thread.start();

    return 0;
}

int Logger::stop()
{

}

std::string Logger::generateLogFilename()
{
    // 获取当前时间
    std::time_t currentTime = std::time(nullptr);
    std::tm* localTime = std::localtime(&currentTime);

    // 格式化时间为所需的文件名格式
    std::stringstream filenameStream;
    filenameStream << "log_" << std::setfill('0') << std::setw(4) << (localTime->tm_year + 1900)
                   << std::setw(2) << (localTime->tm_mon + 1) << std::setw(2) << localTime->tm_mday
                   << std::setw(2) << localTime->tm_hour << '.' << std::setw(2) << localTime->tm_min
                   << ".log";

    std::string logFilename = filenameStream.str();

    return logFilename;
}


#endif