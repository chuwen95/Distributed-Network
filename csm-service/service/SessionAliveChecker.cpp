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
            std::unique_lock<std::mutex> ulock(x_clientLastRecvTime);

            for (auto iter = m_clientLastRecvTime.begin(); iter != m_clientLastRecvTime.end(); ++iter)
            {
                if (iter->second.first == iter->second.second)
                {
                    LOG->write(utilities::LogType::Log_Info, FILE_INFO, "client offline, fd: ", iter->first);
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
    m_thread->setName("cli_alive_chk");

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

int SessionAliveChecker::addClient(const int fd)
{
    std::unique_lock<std::mutex> ulock(x_clientLastRecvTime);

    auto iter = m_clientLastRecvTime.find(fd);
    if (m_clientLastRecvTime.end() != iter)
    {
        LOG->write(utilities::LogType::Log_Error, FILE_INFO, "client already exist, fd: ", fd);
        return -1;
    }

    m_clientLastRecvTime.emplace(fd, std::make_pair(0, 0));

    LOG->write(utilities::LogType::Log_Info, FILE_INFO, "add client successfully, fd: ", fd);

    return 0;
}

int SessionAliveChecker::removeClient(const int fd)
{
    std::unique_lock<std::mutex> ulock(x_clientLastRecvTime);

    auto iter = m_clientLastRecvTime.find(fd);
    if (m_clientLastRecvTime.end() == iter)
    {
        LOG->write(utilities::LogType::Log_Error, FILE_INFO, "client not exist, fd: ", fd);
        return -1;
    }
    m_clientLastRecvTime.erase(iter);

    LOG->write(utilities::LogType::Log_Info, FILE_INFO, "client remove successfully, fd: ", fd);

    return 0;
}

int SessionAliveChecker::refreshClientLastRecvTime(const int fd)
{
    std::unique_lock<std::mutex> ulock(x_clientLastRecvTime);

    auto iter = m_clientLastRecvTime.find(fd);
    if (m_clientLastRecvTime.end() == iter)
    {
        LOG->write(utilities::LogType::Log_Error, FILE_INFO, "client not exist, fd: ", fd);
        return -1;
    }
    iter->second.second = (iter->second.second + 1) % c_aliveCountCircle;

    return 0;
}