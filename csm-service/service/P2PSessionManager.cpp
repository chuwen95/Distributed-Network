//
// Created by ChuWen on 2024/8/8.
//

#include "P2PSessionManager.h"
#include "csm-utilities/Logger.h"

using namespace csm::service;

int P2PSessionManager::addSession(const int fd, P2PSession::Ptr p2pSession)
{
    std::unique_lock<std::mutex> ulock(x_sessions);

    if(m_sessions.end() != m_sessions.find(fd))
    {
        return -1;
    }

    m_sessions[fd] = std::move(p2pSession);

    return 0;
}

int P2PSessionManager::removeSession(const int fd)
{
    std::unique_lock<std::mutex> ulock(x_sessions);

    auto iter = m_sessions.find(fd);
    if(m_sessions.end() == iter)
    {
        return -1;
    }

    m_sessions.erase(iter);

    return 0;
}

P2PSession::Ptr P2PSessionManager::session(const int fd)
{
    std::unique_lock<std::mutex> ulock(x_sessions);

    auto iter = m_sessions.find(fd);
    if(m_sessions.end() == iter)
    {
        return nullptr;
    }

    return iter->second;
}

bool P2PSessionManager::isSessionExist(const int fd)
{
    std::unique_lock<std::mutex> ulock(x_sessions);

    return m_sessions.end() != m_sessions.find(fd);
}

std::size_t P2PSessionManager::sessionSize()
{
    std::unique_lock<std::mutex> ulock(x_sessions);

    return m_sessions.size();
}

