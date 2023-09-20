//
// Created by root on 9/6/23.
//

#ifndef TCPSERVER_LOGGER_H
#define TCPSERVER_LOGGER_H

#include "libcommon/Common.h"
#include "libcomponents/Thread.h"
#include "libcomponents/Singleton.h"

template<typename T>
concept ConvertibleToString = requires(T t) {
    { std::to_string(t) } -> std::convertible_to<std::string>;
};

#define FILE_INFO "[", std::string(__FILE__).substr(std::string(__FILE__).find_last_of("/") + 1), " ", __FUNCTION__, " ", __LINE__, "]    "

namespace components
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
        Logger();
        ~Logger();

    public:
        int init(const std::string_view path, const std::size_t bufferSize);

        int uninit();

        int start();

        int stop();

        void setLogLevel(const LogType level);

        void setConsoleOutput(bool enable);

        template<typename... Args>
        int write(const LogType logType, Args... args)
        {
            if(logType < m_logLevel)
            {
                return 0;
            }

            std::string result = getLogTypeString(logType);
            mergeToString(result, args...);

            {
                std::unique_lock<std::mutex> ulock(x_buffer);
                m_buffer.insert(m_buffer.end(), result.begin(), result.end());
                m_buffer.insert(m_buffer.end(), '\n');
            }

            m_bufferCv.notify_one();

            if(true == m_consoleOutput)
            {
                printf("%s\n", result.c_str());
            }

            return 0;
        }

    private:
        template<typename T>
        std::string toString(const T& value)
        {
            if constexpr (ConvertibleToString<T>)
            {
                return std::to_string(value);
            }
            else
            {
                return value;
            }
        }

        template<typename T, typename... Args>
        void mergeToString(std::string& result, const T& value, const Args&... args)
        {
            result += toString(value);
            if constexpr (sizeof...(args) > 0)
            {
                mergeToString(result, args...);
            }
        }

        std::string getLogTypeString(const LogType type)
        {
            switch(type)
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
        std::string m_path;
        std::ofstream m_file;

        std::mutex x_buffer;
        std::condition_variable m_bufferCv;
        std::vector<char> m_buffer;

        std::atomic_bool m_isTerminate{false};
        components::Thread m_thread;

        bool m_consoleOutput{false};
        LogType m_logLevel{LogType::Log_Info};
    };

} // components

#endif //TCPSERVER_LOGGER_H
