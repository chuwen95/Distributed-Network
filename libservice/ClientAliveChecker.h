//
// Created by root on 9/9/23.
//

#ifndef TCPNETWORK_CLIENTALIVECHECKER_H
#define TCPNETWORK_CLIENTALIVECHECKER_H

#include "libcommon/Common.h"
#include "libcomponents/Thread.h"
#include "libcomponents/CellTimestamp.h"

namespace service
{

    class ClientAliveChecker
    {
    public:
        ClientAliveChecker();
        ~ClientAliveChecker();

    public:
        int init(const std::function<void(const std::vector<int>&)> offlinefdsCallback);

        int uninit();

        int start();

        int stop();

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
         * @brief 获取c_aliveTimeout时间内没有收到任何数据的客户端,
         *              主要是SlaveReactor调用该函数，踢掉这些可能掉线的客户端
         *
         * @param offlinefds
         * @return
         */
        int getOfflineClient(std::vector<int>& offlinefds);

        /**
         * @brief 客户端收到了数据，更新最后收到数据的时间戳
         *
         * @param fd
         * @return
         */
        int refreshClientLastRecvTime(const int fd);

    private:
        std::mutex x_clientLastRecvTime;
        std::unordered_map<int, components::CellTimestamp::Ptr> m_clientLastRecvTime;

        components::Thread m_thread;

        std::function<void(const std::vector<int>&)> m_offlinefdsCallback;
    };

} // server

#endif //TCPNETWORK_CLIENTALIVECHECKER_H
