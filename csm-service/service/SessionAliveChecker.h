//
// Created by ChuWen on 9/9/23.
//

#ifndef SESSIONALIVECHECKER_H
#define SESSIONALIVECHECKER_H

#include "csm-common/Common.h"
#include "csm-utilities/Thread.h"

namespace csm
{

    namespace service
    {

        class SessionAliveChecker
        {
        public:
            using Ptr = std::shared_ptr<SessionAliveChecker>;

            SessionAliveChecker() = default;
            ~SessionAliveChecker() = default;

        public:
            int init();

            int start();

            int stop();

            void setTimeoutHandler(const std::function<void(const std::vector<int>&)> handler);

            /**
             * @brief 添加要监测是否在线的客户端
             *
             * @param fd
             * @return
             */
            int addClient(const int fd);

            /**
             * @brief 移除要监测是否在线的客户端
             *
             * @param fd
             * @return
             */
            int removeClient(const int fd);

            /**
             * @brief 客户端收到了数据，更新最后收到数据的时间戳
             *
             * @param fd
             * @return
             */
            int refreshClientLastRecvTime(const int fd);

        private:
            std::mutex x_clientLastRecvTime;
            std::unordered_map<int, std::pair<std::uint32_t, std::uint32_t>> m_clientLastRecvTime;

            utilities::Thread::Ptr m_thread;
            std::function<void(const std::vector<int> &)> m_timeoutHandler;
        };

    } // server

}

#endif //SESSIONALIVECHECKER_H
