//
// Created by ChuWen on 9/4/23.
//
#include "Thread.h"

using namespace csm::utilities;

Thread::Thread(const std::function<void()>& func, const std::uint32_t interval, const std::string_view threadName) :
    m_threadFunc(func), m_interval(interval), m_threadName(threadName)
{
}

Thread::~Thread()
{
    stop();
}

void Thread::start()
{
    std::unique_lock<std::mutex> ulock(x_startStop);

    if (true == m_isRunning)
    {
        return;
    }

    const auto expression = [this]() {
#ifdef __linux__
        pthread_setname_np(pthread_self(), m_threadName.c_str());
#endif

        while (true == m_isRunning)
        {
            m_threadFunc();

            if (m_interval > 0)
            {
                std::unique_lock<std::mutex> ulock(x_mutex);
                m_cv.wait_for(ulock, std::chrono::milliseconds(m_interval));
            }
        }
    };
    m_isRunning = true;
    m_thread = std::make_unique<std::thread>(expression);
}

void Thread::stop()
{
    std::unique_lock<std::mutex> ulock(x_startStop);

    if (false == m_isRunning)
    {
        return;
    }

    m_isRunning = false;
    m_cv.notify_one();
    m_thread->join();
}