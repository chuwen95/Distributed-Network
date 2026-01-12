//
// Created by ChuWen on 9/4/23.
//

#ifndef TCPSERVER_THREADPOOL_H
#define TCPSERVER_THREADPOOL_H

#include <future>
#include <functional>

#include "concurrentqueue/moodycamel/blockingconcurrentqueue.h"

namespace csm
{

    namespace utilities
    {

        class ThreadPool
        {
        public:
            using ThreadPoolTask = std::function<void()>;

        public:
            using Ptr = std::shared_ptr<ThreadPool>;

            /**
             * @param threadNum  线程池线程数量
             * @param threadName    线程池线程名
             */
            ThreadPool(std::size_t threadNum, std::string_view threadName);
            ~ThreadPool() = default;

        public:
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
            auto push(Func &&func, Args &&... args) -> std::future<decltype(func(args...))>
            {
                using RetType = decltype(func(args...));
                auto task = std::make_shared<std::packaged_task<RetType()>>(std::bind(std::forward<Func>(func), std::forward<Args>(args)...));

                ThreadPoolTask threadPoolTask = [task]() { (*task)(); };
                m_tasks.enqueue(threadPoolTask);

                return task->get_future();
            }

        private:
            /**
             * @brief   设置整个线程池终止标志
             *
             * @param isTerminate   线程池终止标志
             */
            void setIsTerminate(bool isTerminate);

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
            moodycamel::BlockingConcurrentQueue<ThreadPoolTask> m_tasks;
        };

    }

}

#endif //TCPSERVER_THREADPOOL_H
