//
// Created by ChuWen on 9/4/23.
//
#include "Thread.h"

using namespace csm::utilities;

Thread::~Thread()
{
    stop();
}

void Thread::setFunc(std::function<void()> func)
{
    m_func = std::move(func);
}

void Thread::setInterval(std::uint32_t interval)
{
    m_interval = interval;
}

void Thread::setName(std::string_view name)
{
    m_name = name;
}

int Thread::start()
{
    if (nullptr == m_func)
    {
        return -1;
    }

    std::unique_lock<std::mutex> ulock(x_startStop);

    if (true == m_isRunning)
    {
        return -1;
    }

    const auto expression = [this]() {
#ifdef __linux__
        pthread_setname_np(pthread_self(), m_name.c_str());
#endif

        while (true == m_isRunning)
        {
            m_func();

            if (m_interval > 0)
            {
                std::unique_lock<std::mutex> ulock(x_mutex);
                m_cv.wait_for(ulock, std::chrono::milliseconds(m_interval));
            }
        }
    };
    m_isRunning = true;
    m_thread = std::make_unique<std::thread>(expression);

    return 0;
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