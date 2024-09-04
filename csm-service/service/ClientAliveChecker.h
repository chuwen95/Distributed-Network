//
// Created by ChuWen on 9/9/23.
//

#ifndef TCPNETWORK_CLIENTALIVECHECKER_H
#define TCPNETWORK_CLIENTALIVECHECKER_H

#include "csm-common/Common.h"
#include "csm-utilities/Thread.h"
#include "csm-utilities/Timestamp.h"

namespace csm
{

    namespace service
    {

        class ClientAliveChecker
        {
        public:
            using Ptr = std::shared_ptr<ClientAliveChecker>;

            ClientAliveChecker() = default;
            ~ClientAliveChecker() = default;

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

            utilities::Thread m_thread;
            std::function<void(const std::vector<int> &)> m_timeoutHandler;
        };

    } // server

}

#endif //TCPNETWORK_CLIENTALIVECHECKER_H
