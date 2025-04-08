//
// Created by ChuWen on 9/4/23.
//

#ifndef TCPSERVER_THREAD_H
#define TCPSERVER_THREAD_H

#include "csm-common/Common.h"

namespace csm
{

    namespace utilities
    {

        class Thread
        {
        public:
            using Ptr = std::shared_ptr<Thread>;

            Thread(const std::function<void()>& func, std::uint32_t interval = 0, std::string_view threadName = "");
            ~Thread();

        public:
            void start();

            void stop();

        private:
            // 线程名
            std::string m_threadName;
            // 每次执行线程体的间隔时间
            int m_interval;
            // 线程体
            std::function<void()> m_threadFunc;

            std::mutex x_startStop;

            std::mutex x_mutex;
            std::condition_variable m_cv;

            bool m_isRunning{false};
            std::unique_ptr<std::thread> m_thread;
        };

    }

}


#endif //TCPSERVER_THREADPACKET_H
