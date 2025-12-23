//
// Created by ChuWen on 9/6/23.
//

#include "SessionDispatcher.h"

#include <algorithm>
#include <google/protobuf/message.h>

#include "csm-utilities/Logger.h"

using namespace csm::service;

SessionDispatcher::SessionDispatcher(std::size_t slaveReactorSize) : m_slaveReactorSize(slaveReactorSize)
{
    std::fill_n(std::back_inserter(m_slaveReactorSessionSize), slaveReactorSize, 0);
}

int SessionDispatcher::init()
{
    const auto expression = [this]()
    {
        if (true == m_isTerminate)
        {
            return;
        }

        SessionInfo::Ptr sessionInfo{nullptr};
        m_p2pSessionsQueue.wait_dequeue(sessionInfo);

        if (true == m_isTerminate)
        {
            return;
        }

        auto iter = std::ranges::min_element(m_slaveReactorSessionSize.begin(), m_slaveReactorSessionSize.end());
        {
            // 记录客户端fd所在的SlaveRactor
            std::unique_lock<std::mutex> ulock(x_sessionIdSlaveReactorIndex);
            m_sessionIdSlaveReactorIndex[sessionInfo->p2pSession->sessionId()] = *iter;
        }

        sessionInfo->callback(*iter);
        ++m_slaveReactorSessionSize[*iter];
    };
    m_thread = std::make_unique<utilities::Thread>(expression, 1, "session_dispa");

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

int SessionDispatcher::addSession(P2PSession::Ptr p2pSession, std::function<void(std::size_t slaveReactorIndex)> callback)
{
    m_p2pSessionsQueue.try_emplace(std::make_shared<SessionInfo>(p2pSession, std::move(callback)));

    return 0;
}

int SessionDispatcher::removeSessionIdSlaveReactorRelation(const SessionId sessionId)
{
    std::unique_lock<std::mutex> ulock(x_sessionIdSlaveReactorIndex);

    auto iter = m_sessionIdSlaveReactorIndex.find(sessionId);
    if (m_sessionIdSlaveReactorIndex.end() == iter)
    {
        return -1;
    }

    // 移除关联关系
    m_sessionIdSlaveReactorIndex.erase(sessionId);

    return 0;
}

int SessionDispatcher::getSlaveReactorIndexBySessionId(const SessionId sessionId)
{
    std::unique_lock<std::mutex> ulock(x_sessionIdSlaveReactorIndex);

    auto iter = m_sessionIdSlaveReactorIndex.find(sessionId);
    if (m_sessionIdSlaveReactorIndex.end() == iter)
    {
        return -1;
    }

    return iter->second;
}