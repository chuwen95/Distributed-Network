//
// Created by ChuWen on 2024/8/8.
//

#include "SessionDestroyer.h"

using namespace csm::service;

int SessionDestroyer::init()
{
    const auto expression = [this](){
        {
            std::unique_lock<std::mutex> ulock(x_waitingDestroySessionInfos);
            m_waitingDestroySessionInfosCv.wait(ulock, [&](){ return false == m_waitingDestroySessionInfos.empty(); });
        }

        while(true)
        {
            if(nullptr != m_destoryHandler)
            {
                std::pair<int, int> sessionInfo;
                {
                    std::unique_lock<std::mutex> ulock(x_waitingDestroySessionInfos);

                    sessionInfo = m_waitingDestroySessionInfos.front();
                    m_waitingDestroySessionInfos.pop();
                }

                m_destoryHandler(sessionInfo.first, sessionInfo.second);
            }

            std::this_thread::sleep_for(std::chrono::milliseconds(m_destroyInterval));
        }
    };
    m_thread.init(expression, 0, "destoryer");

    return 0;
}

int SessionDestroyer::start()
{
    m_thread.start();

    return 0;
}

int SessionDestroyer::stop()
{
    m_destroyInterval = 0;
    m_thread.stop();

    return 0;
}

int SessionDestroyer::addSession(const int fd, const int flag)
{
    std::unique_lock<std::mutex> ulock(x_waitingDestroySessionInfos);
    m_waitingDestroySessionInfos.emplace(fd, flag);

    return 0;
}

int SessionDestroyer::addSessions(const std::vector<std::pair<int, int>>& sessionInfos)
{
    std::unique_lock<std::mutex> ulock(x_waitingDestroySessionInfos);
    for(const std::pair<int, int> &sessionInfo : sessionInfos)
    {
        m_waitingDestroySessionInfos.emplace(sessionInfo.first, sessionInfo.second);
    }

    return 0;
}

void SessionDestroyer::setHandler(const std::function<int(const int, const int)> handler)
{
    m_destoryHandler = std::move(handler);
}
