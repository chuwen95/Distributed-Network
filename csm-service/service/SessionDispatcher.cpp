//
// Created by ChuWen on 9/6/23.
//

#include "SessionDispatcher.h"

using namespace csm::service;

SessionDispatcher::SessionDispatcher(std::size_t redispatchInterval, const std::string& hostId, std::size_t slaveReactorSize) :
        m_redispatchInterval(redispatchInterval), m_id(hostId), m_slaveReactorSize(slaveReactorSize)
{
    for(std::size_t i = 0; i < m_slaveReactorSize; ++i)
    {
        m_slaveReactorFdSize.emplace_back(std::make_unique<std::atomic_uint32_t>());
    }
}

SessionDispatcher::~SessionDispatcher()
{}

int SessionDispatcher::init()
{
    const auto expression = [this]() {
        if(true == m_isTerminate)
        {
            return;
        }

        SessionInfo::Ptr sessionInfo{nullptr};
        m_p2pSessionsQueue.wait_dequeue(sessionInfo);

        if(nullptr == sessionInfo)
        {
            return;
        }

        {
            // 记录客户端fd所在的SlaveRactor
            std::unique_lock<std::mutex> ulock(x_fdSlaveReactorIndex);
            m_fdSlaveReactorIndex[sessionInfo->fd] = m_slaveReactorIndexWhichHasLeastFd;
        }

        sessionInfo->callback(m_slaveReactorIndexWhichHasLeastFd);

        // 一定间隔后刷新拥有最少数量的SlaveReactor
        static std::size_t s_refreshTime{0};

        ++s_refreshTime;
        if (0 == s_refreshTime % m_redispatchInterval)
        {
            // 寻找管理最少client fd的SlaveReactor的index
            std::size_t maxSize = std::numeric_limits<std::size_t>::max();
            for (std::size_t i = 0; i < m_slaveReactorSize; ++i)
            {
                if (maxSize > *(m_slaveReactorFdSize[i]))
                {
                    maxSize = *(m_slaveReactorFdSize[i]);
                    m_slaveReactorIndexWhichHasLeastFd = i;
                }
            }
            s_refreshTime = 0;
        }
    };
    m_thread = std::make_shared<utilities::Thread>();
    m_thread->setFunc(expression);
    m_thread->setInterval(1);
    m_thread->setName("session_dispa");

    return 0;
}

int SessionDispatcher::start()
{
    m_thread->start();

    return 0;
}

int SessionDispatcher::stop()
{
    m_isTerminate = true;
    m_p2pSessionsQueue.emplace(nullptr);
    m_thread->stop();

    return 0;
}

int SessionDispatcher::addSession(const int fd, std::function<void(const std::size_t slaveReactorIndex)> callback)
{
    m_p2pSessionsQueue.try_emplace(std::make_shared<SessionInfo>(fd, callback));

    return 0;
}

int SessionDispatcher::removeFdSlaveReactorRelation(const int fd)
{
    std::unique_lock<std::mutex> ulock(x_fdSlaveReactorIndex);

    auto iter = m_fdSlaveReactorIndex.find(fd);
    if (m_fdSlaveReactorIndex.end() == iter)
    {
        return -1;
    }

    m_fdSlaveReactorIndex.erase(fd);

    return 0;
}

int SessionDispatcher::getSlaveReactorIndexByFd(const int fd)
{
    std::unique_lock<std::mutex> ulock(x_fdSlaveReactorIndex);

    auto iter = m_fdSlaveReactorIndex.find(fd);
    if (m_fdSlaveReactorIndex.end() == iter)
    {
        return -1;
    }

    return iter->second;
}