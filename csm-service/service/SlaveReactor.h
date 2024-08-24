//
// Created by ChuWen on 9/5/23.
//

#ifndef TCPSERVER_SLAVEREACTOR_H
#define TCPSERVER_SLAVEREACTOR_H

#include "csm-common/Common.h"
#include "csm-utilities/Thread.h"
#include "TcpSession.h"
#include "TcpSessionManager.h"

namespace csm
{

    namespace service
    {

        class SlaveReactor
        {
        public:
            using Ptr = std::shared_ptr<SlaveReactor>;

            /**
             * @brief 参数主要是用于设置线程名
             * @param id
             */
            SlaveReactor(const int reactorId, const std::string& hostId, TcpSessionManager::Ptr tcpSessionManager);
            ~SlaveReactor();

        public:
            int init();

            int start();

            int stop();

            /**
             * @brief 添加客户端
             *
             * @param fd
             * @return
             */
            int addClient(const int fd);

            /**
             * @brief 设置心跳刷新回调
             * @param handler
             * @return
             */
            void setHeartbeatRefreshHandler(const std::function<int(const int fd)> handler);

            /**
             * @brief 设置链接断开处理回调
             * @param handler
             */
            void setDisconnectHandler(const std::function<int(const int fd)> handler);

            /**
             * @brief 设置TcpSession有数据需要解码处理的回调函数
             * @param handler
             */
            void setSessionDataHandler(const std::function<int(const int fd, const char* data, const std::size_t dataLen)> handler);

            /**
             * @brief 写入待发送的数据到writeBuffer，如果writeBuffer的可用空间小于要写入的数据大小，则返回-1，客户端需重新尝试写入
             * 这里不能writeBuffer可用空间还剩多少就写入多少，防止多个客户端同时写入的时候包数据混乱
             *
             * @param fd 客户端fd
             * @param data 数据指针
             * @param size 数据大小
             * @return
             */
            int sendData(const int fd, const char *data, const std::size_t size);

            int sendData(const int fd, const std::vector<char> data);

            /**
             * 将fd从SlaveReactor中移除
             * @param fd
             * @return
             */
            int removeClient(const int fd);

            /**
             * @brief 断开某fd的连接
             *
             * @param fd
             * @param flag SlaveReactor内部使用
             * @return
             */
            int disconnectClient(const int fd, const int flag = 0);

        private:
            // SlaveReactor的id
            int m_reactorId;
            // TcpSession管理器
            TcpSessionManager::Ptr m_tcpSessionManager;

            std::atomic_bool m_isTerminate{false};
            utilities::Thread m_thread;

            int m_epfd{-1};
            std::vector<char> m_recvBuffer;

            // 心跳刷新回调
            std::function<int(const int fd)> m_heartRefreshHandler;
            // 链接断开处理回调
            std::function<int(const int fd)> m_disconnectHandler;
            // 收到数据后对数据进行处理的回调
            std::function<int(const int fd, const char* data, const std::size_t dataLen)> m_sessionDataHandler;

            // 多个线程同时写入待发送的数据，需要加锁
            std::mutex x_writeBuffer;
        };

    }

}

#endif //TCPSERVER_SLAVEREACTOR_H