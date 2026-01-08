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
            explicit Thread(std::function<void(const std::stop_token& st)> func, std::uint32_t interval = 1, std::string name = "");
            ~Thread();

        public:
            void start();

            void stop();

        private:
            // 线程体
            std::function<void(const std::stop_token& st)> m_func{nullptr};
            // 每次执行线程体的间隔时间
            std::uint32_t m_interval{0};
            // 线程名
            std::string m_name;

            std::mutex x_startStop;
            bool m_isStart{false};

            std::mutex x_mutex;
            std::condition_variable m_cv;

            std::jthread m_thread;
        };

    }

}


#endif //TCPSERVER_THREADPACKET_H
