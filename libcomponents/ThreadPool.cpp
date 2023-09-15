//
// Created by root on 9/4/23.
//

#include "ThreadPool.h"
#include "libcomponents/Logger.h"

namespace components
{

    ThreadPool::ThreadPool()
    {

    }

    ThreadPool::~ThreadPool()
    {
    }

    int ThreadPool::init(const int size, const std::string_view threadName)
    {
        m_threadNum = size;
        m_threadName = threadName;

        return 0;
    }

    int ThreadPool::uninit()
    {
        return 0;
    }

    std::size_t ThreadPool::getTaskNum()
    {
        std::unique_lock<std::mutex> ulock(x_tasks);

        return m_tasks.size();
    }

    int ThreadPool::start()
    {
        std::unique_lock<std::mutex> ulock(x_isTerminate);

        const auto expression = [this](const int id)
        {
            std::string tname = m_threadName + "-" + std::to_string(id);
            pthread_setname_np(pthread_self(), tname.c_str());

            while(false == isTerminate())
            {
                ThreadPoolTask task;
                bool ret = getTask(task);
                if(true == ret)
                {
                    ++m_runningTaskNum;
                    task();
                    --m_runningTaskNum;
                }

                std::unique_lock<std::mutex> ulock(x_tasks);
                if(0 == m_runningTaskNum && true == m_tasks.empty())
                {
                    m_taskCv.notify_all();
                }
            }
        };

        setIsTerminate(false);
        for(int i = 0; i < m_threadNum; ++i)
        {
            m_threads.emplace_back(std::make_unique<std::thread>(expression, i));
        }

        return 0;
    }

    int ThreadPool::stop()
    {
        std::unique_lock<std::mutex> ulock(x_isTerminate);

        if(true == isTerminate())
        {
            return 0;
        }

        m_taskCv.notify_all();

        for(auto& thread : m_threads)
        {
            thread->join();
        }

        setIsTerminate(true);

        return 0;
    }

    void ThreadPool::waitForAllDone(const int millseconds)
    {
        std::unique_lock<std::mutex> ulock(x_tasks);

        if(true == m_tasks.empty() && 0 == m_runningTaskNum)
        {
            return;
        }

        if(millseconds < 0)
        {
            m_taskCv.wait(ulock, [this](){ return true == m_tasks.empty() && 0 == m_runningTaskNum; });
        }
        else
        {
            m_taskCv.wait_for(ulock, std::chrono::milliseconds(millseconds), [this](){ return true == m_tasks.empty() && 0 == m_runningTaskNum; });
        }
    }

    bool ThreadPool::getTask(ThreadPool::ThreadPoolTask& task)
    {
        std::unique_lock<std::mutex> ulock(x_tasks);

        if(true == m_tasks.empty())
        {
            m_taskCv.wait(ulock, [this](){ return false == m_tasks.empty(); });
        }

        if(true == isTerminate())
        {
            return false;
        }

        task = std::move(m_tasks.front());
        m_tasks.pop_front();

        return true;
    }

    void ThreadPool::setIsTerminate(const bool isTerminate)
    {
        m_isTerminate = isTerminate;
    }

    bool ThreadPool::isTerminate()
    {
        return m_isTerminate;
    }

}