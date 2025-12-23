//
// Created by ChuWen on 9/4/23.
//
#include "Thread.h"

#include "Logger.h"

using namespace csm::utilities;

Thread::Thread(std::function<void()> func, std::uint32_t interval, std::string name) : m_func(std::move(func)),
    m_interval(interval), m_name(name)
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

    m_thread = std::jthread([this](std::stop_token st)
    {
#ifdef __linux__
        pthread_setname_np(pthread_self(), m_name.c_str());
#endif

        while (false == st.stop_requested())
        {
            m_func();

            if (m_interval > 0)
            {
                std::unique_lock<std::mutex> ulock(x_mutex);
                m_cv.wait_for(ulock, std::chrono::milliseconds(m_interval));
            }
        }
    });
    m_isRunning = true;
}

void Thread::stop()
{
    std::unique_lock<std::mutex> ulock(x_startStop);

    if (false == m_isRunning)
    {
        return;
    }

    m_thread.request_stop();
    m_cv.notify_one();
    m_thread.join();

    m_isRunning = false;
}
