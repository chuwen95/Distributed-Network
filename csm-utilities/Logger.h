//
// Created by ChuWen on 9/6/23.
//

#ifndef LOGGER_H
#define LOGGER_H

#include "csm-common/Common.h"
#include "Thread.h"
#include "Singleton.h"
#include "StringTool.h"

#define FILE_INFO "[", __FILE__ , " ", __FUNCTION__, " ", __LINE__, "]    "

namespace csm
{

    namespace utilities
    {

        enum class LogType
        {
            Log_Trace,
            Log_Debug,
            Log_Info,
            Log_Warning,
            Log_Error,
        };

        constexpr std::size_t c_defaultBufferSize{ 32 * 1024 * 1024 };

        class Logger
        {
        public:
            void setEnableFileLog(bool enableFileLog);

            void setLogDirPath(const std::string &logDirPath);

            void setLogBufferSize(std::size_t bufferSize);

            int init();

            int start();

            int stop();

            void setLogLevel(const LogType level);

            void setConsoleOutput(bool enable);

            template<typename... Args>
            int write(const LogType logType, Args... args)
            {
                if (logType < m_logLevel)
                {
                    return 0;
                }

                std::string result = getLogTypeString(logType);
                mergeToString(result, args...);

                if (true == m_enableFileLog)
                {
                    {
                        std::unique_lock<std::mutex> ulock(x_buffer);
                        m_buffer.insert(m_buffer.end(), result.begin(), result.end());
                        m_buffer.insert(m_buffer.end(), '\n');
                    }

                    m_bufferCv.notify_all();
                }

                if (true == m_consoleOutput)
                {
                    std::cout << result << std::endl;
                }

                return 0;
            }

        private:
            template<typename T, typename... Args>
            void mergeToString(std::string &result, const T &value, const Args &... args)
            {
                result += convertToString(value);
                if constexpr (sizeof...(args) > 0)
                {
                    mergeToString(result, args...);
                }
            }

            std::string getLogTypeString(const LogType type)
            {
                switch (type)
                {
                    case LogType::Log_Trace:
                        return "[Trace]";
                    case LogType::Log_Debug:
                        return "[Debug]";
                    case LogType::Log_Info:
                        return "[Info]";
                    case LogType::Log_Warning:
                        return "[Warning]";
                    case LogType::Log_Error:
                        return "[Error]";
                    default:
                        return "";
                }
            }

        private:
            std::string generateLogFilename();
            std::string generateLogFullFilename(const std::string& logDirPath, const std::string& logFilename);

        private:
            bool m_enableFileLog{ true };
            std::string m_logDirPath{ "./log" };
            std::size_t m_bufferSize{ c_defaultBufferSize };

            std::ofstream m_file;

            std::mutex x_buffer;
            std::condition_variable m_bufferCv;
            std::vector<char> m_buffer;

            std::unique_ptr<Thread> m_thread;

            bool m_consoleOutput{false};
            LogType m_logLevel{LogType::Log_Info};
        };

    } // components

}

#define LOG csm::utilities::Singleton<csm::utilities::Logger>::instance()

/*

#include "concurrentqueue/concurrentqueue.h"

namespace csm
{

    namespace utilities
    {

        enum class LogType
        {
            Log_Trace,
            Log_Debug,
            Log_Info,
            Log_Warning,
            Log_Error,
        };

        class Logger
        {
        public:
            Logger(const bool enableFileLog, const std::string &path);
            ~Logger() = default;

        public:
            int init();

            int start();
            int stop();

        private:
            std::string generateLogFilename();

        private:
            bool m_enableFileLog;
            std::string m_path;

            moodycamel::BlockingConcurrentQueue<std::string, 1024> m_logs;
            std::ofstream m_file;

            std::atomic_bool m_isTerminate{false};
            Thread m_thread;

            bool m_consoleOutput{false};
            LogType m_logLevel{LogType::Log_Info};
        };

    }

}

*/

#endif //TCPSERVER_LOGGER_H
