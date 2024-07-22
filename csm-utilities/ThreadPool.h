//
// Created by root on 9/4/23.
//

#ifndef TCPSERVER_THREADPOOL_H
#define TCPSERVER_THREADPOOL_H

#include "csm-common/Common.h"
#include "concurrentqueue/blockingconcurrentqueue.h"

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

            ThreadPool();

            ~ThreadPool();

        public:
            /**
             * @brief   初始化线程池
             *
             * @param threadNum  线程池线程数量
             * @param threadName    线程池线程名
             * @return
             */
            int init(const int threadNum, const std::string_view threadName);

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
            auto push(Func &&func, Args &&... args) -> std::future<decltype(func(args...))>
            {
                if ((m_taskPushNums > 0) && (0 == m_taskPushNums % m_currentTaskPushIndexRefreshInterval))
                {
                    std::size_t maxSize{std::numeric_limits<std::size_t>::max()};
                    for (auto iter = m_taskQueues.begin(); iter != m_taskQueues.end(); ++iter)
                    {
                        auto taskSize = iter->size_approx();
                        if (taskSize < maxSize)
                        {
                            m_currentTaskPushIndex = iter - m_taskQueues.begin();
                            maxSize = taskSize;
                        }
                    }
                    m_taskPushNums = 0;
                }

                using RetType = decltype(func(args...));
                auto task = std::make_shared<std::packaged_task<RetType()>>(
                        std::bind(std::forward<Func>(func), std::forward<Args>(args)...));

                ThreadPoolTask threadPoolTask = [task]() { (*task)(); };
                m_taskQueues[m_currentTaskPushIndex].enqueue(threadPoolTask);

                // 已添加的任务数量+1
                ++m_taskPushNums;

                return task->get_future();
            }

        private:
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
            std::vector<moodycamel::BlockingConcurrentQueue<ThreadPoolTask>> m_taskQueues;

            // 每放入多少个任务刷新一次
            std::atomic_size_t m_taskPushNums{0};
            std::atomic_size_t m_currentTaskPushIndexRefreshInterval{500};
            // 当前任务往哪个队列里放
            std::atomic_size_t m_currentTaskPushIndex{0};
        };

    }

}

#endif //TCPSERVER_THREADPOOL_H
