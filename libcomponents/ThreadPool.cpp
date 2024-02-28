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
        if (size < 1)
        {
            return -1;
        }

        m_threadNum = size;
        m_threadName = threadName;

        m_taskQueues.resize(size);

        return 0;
    }

    int ThreadPool::uninit()
    {
        return 0;
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
                m_taskQueues[id].wait_dequeue(task);
                task();
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

        setIsTerminate(true);

        for(int i = 0; i < m_threadNum; ++i)
        {
            m_taskQueues[i].enqueue([](){});
        }

        for(auto& thread : m_threads)
        {
            thread->join();
        }

        return 0;
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