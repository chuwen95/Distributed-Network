//
// Created by root on 9/4/23.
//

#ifndef TCPSERVER_THREADPOOL_H
#define TCPSERVER_THREADPOOL_H

#include "libcommon/Common.h"

namespace components
{

    class ThreadPool
    {
    public:
        using ThreadPoolTask = std::function<void()>;

    public:
        using Ptr = std::shared_ptr<ThreadPool>;

        ThreadPool();
        ~ThreadPool();

    public:
        /**
         * @brief   初始化线程池
         *
         * @param size  线程池线程数量
         * @param threadName    线程池线程名
         * @return
         */
        int init(const int size, const std::string_view threadName);

        /**
         *@brief    反初始化线程池
         * @return
         */
        int uninit();

        /**
         * @brief   开启线程池
         */
        int start();

        /**
         * @brief   停止线程池，会等待线程池中每个线程正在运行的任务完成，
         *                  还在工作队列等待的任务会被丢弃
         */
        int stop();

        /**
         * @brief   添加任务到线程池
         * @tparam Func
         * @tparam Args
         * @param func
         * @param args
         * @return
         */
        template<typename Func, typename... Args>
        auto push(Func&& func, Args&&... args) -> std::future<decltype(func(args...))>
        {
            using RetType = decltype(func(args...));
            auto task = std::make_shared<std::packaged_task<RetType()>>(std::bind(std::forward<Func>(func), std::forward<Args>(args)...));

            ThreadPoolTask threadPoolTask = [task](){ (*task)(); };
            {
                std::unique_lock<std::mutex> ulock(x_tasks);
                m_tasks.emplace_back(threadPoolTask);
            }
            m_taskCv.notify_one();

            return task->get_future();
        }

        /**
         * @brief   获取当前线程池中的任务数
         *
         * @return  std::size_t 当前线程池任务数量
         */
        std::size_t getTaskNum();

        /**
         * @brief   等待所有任务完成，包括任务队列中的任务
         *
         * @param millseconds   等待时长，-1表示永远等待
         */
        void waitForAllDone(const int millseconds = -1);

    private:
        /**
         * @brief   从任务队列中获取任务，如果没有任务了，会阻塞
         *
         * @param task  任务
         * @return bool     获取任务成功/失败
         */
        bool getTask(ThreadPoolTask& task);

        /**
         * @brief   设置整个线程池终止标志
         *
         * @param isTerminate   线程池终止标志
         */
        void setIsTerminate(const bool isTerminate);

        /**
         * @brief   整个线程池终止标志
         *
         * @return
         */
        bool isTerminate();

    private:
        // 线程池工作线程数量
        std::size_t m_threadNum{1};
        // 工作线程名称，每个工作线程会命名为工作线程名称_id
        std::string m_threadName;

        // 线程池
        std::mutex x_isTerminate;
        bool m_isTerminate{false};
        std::vector<std::unique_ptr<std::thread>> m_threads;

        // 任务队列
        std::mutex x_tasks;
        std::condition_variable m_taskCv;
        std::deque<ThreadPoolTask> m_tasks;

        // 正在线程池中执行的任务数量
        std::atomic_size_t m_runningTaskNum{0};
    };

}

#endif //TCPSERVER_THREADPOOL_H
