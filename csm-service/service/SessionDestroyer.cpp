//
// Created by ChuWen on 2024/8/8.
//

#include "SessionDestroyer.h"

#include "csm-utilities/Logger.h"

using namespace csm::service;

int SessionDestroyer::init()
{
    const auto expression = [this]()
    {
        {
            std::unique_lock<std::mutex> ulock(x_waitingDestroySessionInfos);
            m_waitingDestroySessionInfosCv.wait(
                ulock, [&]() { return 0 == m_destroyInterval || false == m_waitingDestroySessionInfos.empty(); });
        }

        while (true)
        {
            if (nullptr != m_destoryHandler)
            {
                SessionDestoryInfo info;

                {
                    std::unique_lock<std::mutex> ulock(x_waitingDestroySessionInfos);

                    if (true == m_waitingDestroySessionInfos.empty())
                    {
                        break;
                    }

                    info = m_waitingDestroySessionInfos.front();

                    m_waitingDestroySessionInfos.pop();
                }

                m_destoryHandler(info);
            }

            if (0 != m_destroyInterval)
            {
                std::this_thread::sleep_for(std::chrono::milliseconds(m_destroyInterval));
            }
        }
    };
    m_thread = std::make_shared<utilities::Thread>();
    m_thread->setFunc(expression);
    m_thread->setName("destoryer");

    return 0;
}

int SessionDestroyer::start()
{
    m_thread->start();

    return 0;
}

int SessionDestroyer::stop()
{
    m_destroyInterval = 0;
    m_waitingDestroySessionInfosCv.notify_all();
    m_thread->stop();

    return 0;
}

void SessionDestroyer::addSession(const SessionDestoryInfo& session)
{
    std::unique_lock<std::mutex> ulock(x_waitingDestroySessionInfos);

    m_waitingDestroySessionInfos.push(session);
    m_waitingDestroySessionInfosCv.notify_all();
}

void SessionDestroyer::addSessions(const std::vector<SessionDestoryInfo>& infos)
{
    std::unique_lock<std::mutex> ulock(x_waitingDestroySessionInfos);

    for (const SessionDestoryInfo& info : infos)
    {
        m_waitingDestroySessionInfos.emplace(info);
    }
    m_waitingDestroySessionInfosCv.notify_all();
}

void SessionDestroyer::setHandler(std::function<void(const SessionDestoryInfo& info)> handler)
{
    m_destoryHandler = std::move(handler);
}