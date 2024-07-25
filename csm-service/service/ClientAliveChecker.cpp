//
// Created by root on 9/9/23.
//

#include "ClientAliveChecker.h"
#include "csm-utilities/Logger.h"

using namespace csm::service;

// ClientAliveChecker任务运行间隔，每间隔一段时间运行一次检查
constexpr std::size_t c_aliveCheckInterval{3000};
// 若c_aliveTimeout时间内没有收到任何数据，判定客户端掉线
constexpr std::size_t c_aliveTimeout{30000};

ClientAliveChecker::ClientAliveChecker()
{}

ClientAliveChecker::~ClientAliveChecker()
{}

int ClientAliveChecker::init(const std::function<void(const std::vector<int> &)> offlinefdsCallback)
{
    m_offlinefdsCallback = std::move(offlinefdsCallback);

    const auto expression = [this]() {
        std::vector<int> offlinefds;

        std::unique_lock<std::mutex> ulock(x_clientLastRecvTime);
        for (auto iter = m_clientLastRecvTime.begin(); iter != m_clientLastRecvTime.end(); ++iter)
        {
            if (iter->second->getElapsedTimeInMilliSec() > c_aliveTimeout)
            {
                LOG->write(utilities::LogType::Log_Info, FILE_INFO, "client offline, fd: ", iter->first);
                offlinefds.emplace_back(iter->first);
            }
        }

        if (nullptr != m_offlinefdsCallback)
        {
            m_offlinefdsCallback(offlinefds);
        }
    };
    m_thread.init(expression, c_aliveCheckInterval, "cli_check");

    return 0;
}

int ClientAliveChecker::uninit()
{
    m_thread.uninit();

    return 0;
}

int ClientAliveChecker::start()
{
    m_thread.start();

    return 0;
}

int ClientAliveChecker::stop()
{
    m_thread.stop();

    return 0;
}

int ClientAliveChecker::addClient(const int fd)
{
    std::unique_lock<std::mutex> ulock(x_clientLastRecvTime);

    auto iter = m_clientLastRecvTime.find(fd);
    if (m_clientLastRecvTime.end() != iter)
    {
        LOG->write(utilities::LogType::Log_Error, FILE_INFO, "client already exist, fd: ", fd);
        return -1;
    }

    utilities::CellTimestamp::Ptr cellTimestamp = std::make_shared<utilities::CellTimestamp>();
    cellTimestamp->update();
    m_clientLastRecvTime.emplace(fd, cellTimestamp);

    LOG->write(utilities::LogType::Log_Info, FILE_INFO, "add client successfully, fd: ", fd);

    return 0;
}

int ClientAliveChecker::removeClient(const int fd)
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

int ClientAliveChecker::refreshClientLastRecvTime(const int fd)
{
    std::unique_lock<std::mutex> ulock(x_clientLastRecvTime);
    auto iter = m_clientLastRecvTime.find(fd);
    if (m_clientLastRecvTime.end() == iter)
    {
        LOG->write(utilities::LogType::Log_Error, FILE_INFO, "client not exist, fd: ", iter->first);
        return -1;
    }
    iter->second->update();
    return 0;
}