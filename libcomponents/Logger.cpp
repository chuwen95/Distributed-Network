//
// Created by root on 9/6/23.
//

#include "Logger.h"

namespace components
{

    Logger::Logger()
    {}

    Logger::~Logger()
    {}

    int Logger::init(const bool enableFileLog, const std::string& path, const std::size_t bufferSize)
    {
        m_enableFileLog = enableFileLog;
        if (true == m_enableFileLog)
        {
            m_file.open(path, std::ios::out);
            if (false == m_file.is_open())
            {
                std::cerr << "open log file failed: " << path << ", error state: " << m_file.rdstate() << std::endl;
                return -1;
            }

            m_buffer.resize(bufferSize);

            const auto expression = [this]()
            {
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
            };
            m_thread.init(expression, 0, "logger");
        }

        return 0;
    }

    int Logger::uninit()
    {
        m_file.close();

        return 0;
    }

    int Logger::start()
    {
        if(true == m_enableFileLog)
        {
            m_thread.start();
        }

        return 0;
    }

    int Logger::stop()
    {
        if(true == m_enableFileLog)
        {
            m_isTerminate = true;
            m_bufferCv.notify_one();
            m_thread.stop();
        }

        return 0;
    }

    void Logger::setLogLevel(const components::LogType level)
    {
        m_logLevel = level;
    }

    void Logger::setConsoleOutput(bool enable)
    {
        m_consoleOutput = enable;
    }

} // components