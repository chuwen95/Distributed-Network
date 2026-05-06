//
// Created by ChuWen on 9/5/23.
//

#ifndef TCPSERVER_SLAVEREACTOR_H
#define TCPSERVER_SLAVEREACTOR_H

#include <queue>
#include <shared_mutex>

#include "csm-utilities/Thread.h"
#include "P2PSession.h"

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
            explicit SlaveReactor(int reactorId);
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
            int addSession(int fd, P2PSession::Ptr p2pSession);

            /**
             * @brief 设置链接断开处理回调
             * @param handler
             */
            void setDisconnectHandler(std::function<void(SessionId sessionId, P2PSession::WPtr)> handler);

            /**
             * @brief 设置P2PSession有数据需要解码处理的回调函数
             * @param handler
             */
            void setSessionDataHandler(std::function<void(SessionId sessionId, P2PSession::WPtr p2pSessionWeakPtr,
                                                          const char* data,
                                                          std::size_t dataLen)> handler);

            /**
             * @brief 写入待发送的数据到writeBuffer，如果writeBuffer的可用空间小于要写入的数据大小，则返回-1，客户端需重新尝试写入
             * 这里不能writeBuffer可用空间还剩多少就写入多少，防止多个客户端同时写入的时候包数据混乱
             *
             * @param fd 客户端fd
             * @param data 数据指针
             * @param size 数据大小
             * @return
             */
            int sendData(SessionId sessionId, std::shared_ptr<std::vector<char>> data);

            int sendData(P2PSession::Ptr p2pSession, std::shared_ptr<std::vector<char>> data);

            /**
             * 将fd从SlaveReactor中移除
             * @param fd
             * @return
             */
            int removeClient(SessionId sessionId, int fd);

            P2PSession::Ptr getP2PSession(SessionId sessionId);

        private:
            void disconnectSession(const P2PSession::Ptr& p2pSession);

            void handleSendTasks();
            void handleSend(const P2PSession::Ptr& p2pSession, const std::shared_ptr<std::vector<char>>& data);

            int createSendTaskEpollFd();

        private:
            // SlaveReactor的id
            int m_reactorId;
            // SessionId => P2PSession::Ptr
            std::shared_mutex x_p2pSessions;
            std::unordered_map<SessionId, P2PSession::Ptr> m_p2pSessions;

            std::unique_ptr<utilities::Thread> m_thread;

            // epfd
            int m_epfd{-1};
            std::vector<char> m_recvBuffer;

            // 链接断开处理回调
            std::function<void(SessionId sessionId, P2PSession::WPtr p2pSessionWeakPtr)> m_disconnectHandler;
            // 收到数据后对数据进行处理的回调
            std::function<void(SessionId sessionId,
                               P2PSession::WPtr p2pSessionWeakPtr, const char* data,
                               std::size_t dataLen)> m_sessionDataHandler;

            struct SendTask
            {
                SendTask(P2PSession::Ptr p, std::shared_ptr<std::vector<char>> d) : p2pSession(p), data(d) {}

                P2PSession::Ptr p2pSession;
                std::shared_ptr<std::vector<char>> data;
            };

            std::mutex x_sendTasks;
            std::queue<SendTask> m_sendTasks;
            int m_sendTaskWakeupFd;
        };
    }
}

#endif //TCPSERVER_SLAVEREACTOR_H
