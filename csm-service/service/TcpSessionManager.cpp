//
// Created by ChuWen on 2024/8/8.
//

#include "TcpSessionManager.h"

using namespace csm::service;

int TcpSessionManager::addTcpSession(const int fd, TcpSession::Ptr tcpSession)
{
    std::unique_lock<std::mutex> x_tcpSessions;

    if(m_tcpSessions.end() != m_tcpSessions.find(fd))
    {
        return -1;
    }

    m_tcpSessions[fd] = std::move(tcpSession);

    return 0;
}

int TcpSessionManager::removeTcpSession(const int fd)
{
    std::unique_lock<std::mutex> x_tcpSessions;

    auto iter = m_tcpSessions.find(fd);
    if(m_tcpSessions.end() == iter)
    {
        return -1;
    }

    m_tcpSessions.erase(iter);

    return 0;
}

TcpSession::Ptr TcpSessionManager::tcpSession(const int fd)
{
    std::unique_lock<std::mutex> x_tcpSessions;

    auto iter = m_tcpSessions.find(fd);
    if(m_tcpSessions.end() == iter)
    {
        return nullptr;
    }

    return iter->second;
}

bool TcpSessionManager::isTcpSessionExist(const int fd)
{
    std::unique_lock<std::mutex> x_tcpSessions;

    return m_tcpSessions.end() != m_tcpSessions.find(fd);
}

