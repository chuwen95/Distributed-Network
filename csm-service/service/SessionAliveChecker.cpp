//
// Created by ChuWen on 9/9/23.
//

#include "SessionAliveChecker.h"
#include "csm-utilities/Logger.h"

using namespace csm::service;

// 若c_aliveTimeout时间内没有收到任何数据，判定客户端掉线
constexpr std::size_t c_aliveTimeout{ 15000 };
constexpr std::size_t c_aliveCountCircle{ 10000000 };

int SessionAliveChecker::init()
{
    const auto expression = [this]() {
        std::vector<int> offlinefds;

        {
            std::unique_lock<std::mutex> ulock(x_sessionLastRecvTime);

            for (auto iter = m_sessionLastRecvTime.begin(); iter != m_sessionLastRecvTime.end(); ++iter)
            {
                if (iter->second.first == iter->second.second)
                {
                    LOG->write(utilities::LogType::Log_Info, FILE_INFO, "session offline, fd: ",
                        iter->first, ", last count: ", iter->second.first, ", current count: ", iter->second.second);
                    offlinefds.emplace_back(iter->first);
                }
                iter->second.first = iter->second.second;
            }
        }

        if (nullptr != m_timeoutHandler)
        {
            m_timeoutHandler(offlinefds);
        }
    };
    m_thread = std::make_shared<utilities::Thread>();
    m_thread->setFunc(expression);
    m_thread->setInterval(c_aliveTimeout);
    m_thread->setName("ses_alive_chk");

    return 0;
}

int SessionAliveChecker::start()
{
    m_thread->start();

    return 0;
}

int SessionAliveChecker::stop()
{
    m_thread->stop();

    return 0;
}

void SessionAliveChecker::setTimeoutHandler(std::function<void(const std::vector<int> &)> handler)
{
    m_timeoutHandler = std::move(handler);
}

int SessionAliveChecker::addSession(const int fd)
{
    std::unique_lock<std::mutex> ulock(x_sessionLastRecvTime);

    auto iter = m_sessionLastRecvTime.find(fd);
    if (m_sessionLastRecvTime.end() != iter)
    {
        LOG->write(utilities::LogType::Log_Error, FILE_INFO, "session already exist, fd: ", fd);
        return -1;
    }

    m_sessionLastRecvTime.emplace(fd, std::make_pair(0, 1));

    LOG->write(utilities::LogType::Log_Info, FILE_INFO, "add session successfully, fd: ", fd);

    return 0;
}

int SessionAliveChecker::removeSession(const int fd)
{
    std::unique_lock<std::mutex> ulock(x_sessionLastRecvTime);

    auto iter = m_sessionLastRecvTime.find(fd);
    if (m_sessionLastRecvTime.end() == iter)
    {
        LOG->write(utilities::LogType::Log_Error, FILE_INFO, "session not exist, fd: ", fd);
        return -1;
    }
    m_sessionLastRecvTime.erase(iter);

    LOG->write(utilities::LogType::Log_Info, FILE_INFO, "session remove successfully, fd: ", fd);

    return 0;
}

int SessionAliveChecker::refreshSessionLastRecvTime(const int fd)
{
    std::unique_lock<std::mutex> ulock(x_sessionLastRecvTime);

    auto iter = m_sessionLastRecvTime.find(fd);
    if (m_sessionLastRecvTime.end() == iter)
    {
        LOG->write(utilities::LogType::Log_Error, FILE_INFO, "session not exist, fd: ", fd);
        return -1;
    }

    iter->second.second = (iter->second.second + 1) % c_aliveCountCircle;
    LOG->write(utilities::LogType::Log_Debug, FILE_INFO, "refresh session last recv time, fd: ", fd, ", value: ", iter->second.second);

    return 0;
}