//
// Created by ChuWen on 9/4/23.
//

#include "ThreadPool.h"
#include "csm-utilities/Logger.h"

using namespace csm::utilities;

ThreadPool::ThreadPool(const std::size_t size, const std::string_view threadName) :
    m_threadNum(size), m_threadName(threadName)
{}

int ThreadPool::init()
{
    if (m_threadNum < 1)
    {
        return -1;
    }

    return 0;
}

int ThreadPool::start()
{
    std::unique_lock<std::mutex> ulock(x_isTerminate);

    const auto expression = [this](const int id) {
        std::string tname = m_threadName + "-" + std::to_string(id);
#ifdef __linux__
        pthread_setname_np(pthread_self(), tname.c_str());
#endif

        while (false == isTerminate())
        {
            ThreadPoolTask task;
            if (true == m_tasks.wait_dequeue_timed(task, 100000))
            {
                task();
            }
        }
    };

    setIsTerminate(false);
    for (int i = 0; i < m_threadNum; ++i)
    {
        m_threads.emplace_back(std::make_unique<std::thread>(expression, i));
    }

    return 0;
}

int ThreadPool::stop()
{
    std::unique_lock<std::mutex> ulock(x_isTerminate);

    if (true == isTerminate())
    {
        return 0;
    }

    setIsTerminate(true);

    for (auto &thread: m_threads)
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