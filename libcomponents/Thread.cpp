//
// Created by root on 9/4/23.
//
#include "Thread.h"

namespace components
{

    Thread::Thread()
    {
    }

    Thread::~Thread()
    {
        stop();
    }

    int Thread::init(const std::function<void()>  threadFunc, const int time, const std::string_view threadName)
    {
        m_threadFunc = std::move(threadFunc);
        m_time = time;
        m_threadName = threadName;

        return 0;
    }

    int Thread::uninit()
    {
        return 0;
    }

    void Thread::start()
    {
        if(true == m_isRunning)
        {
            return;
        }

        const auto expression = [this]()
        {
            pthread_setname_np(pthread_self(), m_threadName.c_str());

            std::mutex m;
            std::condition_variable cv;

            while(true == m_isRunning)
            {
                m_threadFunc();

                if (m_time >= 0)
                {
                    std::unique_lock<std::mutex> ulock(m);
                    cv.wait_for(ulock, std::chrono::milliseconds(m_time), [this](){ return false == m_isRunning; });
                }
            }
        };
        m_isRunning = true;
        m_thread = std::make_unique<std::thread>(expression);
    }

    void Thread::stop()
    {
        if(false == m_isRunning)
        {
            return;
        }

        m_isRunning = false;
        m_thread->join();
    }

}