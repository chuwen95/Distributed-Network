//
// Created by ChuWen on 9/9/23.
//

#include "SessionAliveChecker.h"

#include "P2PSession.h"
#include "csm-utilities/Logger.h"

using namespace csm::service;

// 若c_aliveTimeout时间内没有收到任何数据，判定客户端掉线
constexpr std::size_t c_aliveTimeout{15000};
constexpr std::size_t c_aliveCountCircle{10000000};

int SessionAliveChecker::init()
{
    const auto expression = [this]()
    {
        std::vector<std::pair<SessionId, P2PSession::WPtr>> offlineSessions;

        {
            std::unique_lock<std::mutex> ulock(x_sessions);

            for (auto iter = m_sessions.begin(); iter != m_sessions.end();)
            {
                P2PSession::Ptr p2pSession = iter->second.lock();
                if (nullptr == p2pSession)
                {
                    LOG->write(utilities::LogType::Log_Info, FILE_INFO, "session may already been destoryed, session id: ", iter->first);
                    iter = m_sessions.erase(iter);
                    continue;
                }

                if (true == p2pSession->isHeartbeatTimeout())
                {
                    LOG->write(utilities::LogType::Log_Info, FILE_INFO, "session heartbeat timeout, session id: ", iter->first);
                    offlineSessions.emplace_back(iter->first, iter->second);
                    iter = m_sessions.erase(iter);
                }
                else
                {
                    ++iter;
                }
            }
        }

        if (nullptr != m_timeoutHandler)
        {
            m_timeoutHandler(offlineSessions);
        }
    };
    m_thread = std::make_unique<utilities::Thread>(expression, c_aliveTimeout, "ses_alive_chk");

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

void SessionAliveChecker::setTimeoutHandler(
    std::function<void(const std::vector<std::pair<SessionId, P2PSession::WPtr>> &)> handler)
{
    m_timeoutHandler = std::move(handler);
}

int SessionAliveChecker::addSession(const SessionId sessionId, P2PSession::WPtr p2pSessionWeakPtr)
{
    std::unique_lock<std::mutex> ulock(x_sessions);

    if (m_sessions.end() != m_sessions.find(sessionId))
    {
        LOG->write(utilities::LogType::Log_Error, FILE_INFO, "session already exist, session id: ", sessionId);
        return -1;
    }
    m_sessions.emplace(sessionId, std::move(p2pSessionWeakPtr));

    LOG->write(utilities::LogType::Log_Info, FILE_INFO, "add session successfully, session id: ", sessionId);

    return 0;
}

int SessionAliveChecker::removeSession(const SessionId sessionId)
{
    std::unique_lock<std::mutex> ulock(x_sessions);

    auto iter = m_sessions.find(sessionId);
    if (m_sessions.end() == iter)
    {
        LOG->write(utilities::LogType::Log_Error, FILE_INFO, "session not exist, session id: ", sessionId);
        return -1;
    }
    m_sessions.erase(iter);

    LOG->write(utilities::LogType::Log_Info, FILE_INFO, "session remove successfully, session id: ", sessionId);

    return 0;
}