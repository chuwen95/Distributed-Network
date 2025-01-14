//
// Created by ChuWen on 9/6/23.
//

#include "Logger.h"

using namespace csm::utilities;

#if 1

Logger::Logger()
{}

Logger::~Logger()
{}

int Logger::init(const bool enableFileLog, const std::string &path, const std::size_t bufferSize)
{
    m_enableFileLog = enableFileLog;

    if (true == enableFileLog)
    {
        std::filesystem::create_directory(path);

        std::string logFilename = generateLogFilename();

        std::string logFullFilename;
        if(true == path.empty())
        {
            logFullFilename = "./" + logFilename;
        }
        else
        {
            if(path.back() == '/')
            {
                logFullFilename = path + logFilename;
            }
            else
            {
                logFullFilename = path + "/" + logFilename;
            }
        }

        m_file.open(logFullFilename, std::ios::out);
        if (false == m_file.is_open())
        {
            std::cerr << "open log file failed: " << logFullFilename << ", error state: " << m_file.rdstate() << ", errno: " << errno << ", " << strerror(errno)<< std::endl;
            return -1;
        }

        m_buffer.resize(bufferSize);

        const auto expression = [this]() {
            if (true == m_isTerminate)
            {
                std::this_thread::sleep_for(std::chrono::milliseconds(5));
                return;
            }

            std::vector<char> buffer;
            {
                std::unique_lock<std::mutex> ulock(x_buffer);
                m_bufferCv.wait(ulock, [this]() { return false == m_buffer.empty() || true == m_isTerminate; });

                if (true == m_isTerminate)
                {
                    return;
                }

                m_buffer.swap(buffer);
            }

            m_file.write(buffer.data(), buffer.size());
            m_file.flush();
        };
        m_thread.init(expression, 1, "logger");
    }

    return 0;
}

int Logger::start()
{
    if (true == m_enableFileLog)
    {
        m_thread.start();
    }

    return 0;
}

int Logger::stop()
{
    if (true == m_enableFileLog)
    {
        m_isTerminate = true;
        m_bufferCv.notify_one();
        m_thread.stop();
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