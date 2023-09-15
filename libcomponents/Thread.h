//
// Created by root on 9/4/23.
//

#ifndef TCPSERVER_THREAD_H
#define TCPSERVER_THREAD_H

#include "libcommon/Common.h"

namespace components
{

    class Thread
    {
    public:
        using Ptr = std::shared_ptr<Thread>;

        Thread();
        ~Thread();

        /**
         * @brief   初始化
         * @param threadFunc          [in] 线程体
         * @param time                      [in] 每次执行线程体的间隔时间
         * @param threadName        [in] 线程名
         */
        int init(const std::function<void()> threadFunc, const int time = 0, const std::string_view threadName = "");

        /**
         * @brief   反初始化
         * @return
         */
        int uninit();

        void start();

        void stop();

    private:
        // 线程名
        std::string m_threadName;
        // 每次执行线程体的间隔时间
        int m_time;
        // 线程体
        std::function<void()> m_threadFunc;

        std::atomic_bool m_isRunning{false};
        std::unique_ptr<std::thread> m_thread;
    };

}


#endif //TCPSERVER_THREADPACKET_H
