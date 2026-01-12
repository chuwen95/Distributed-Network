//
// Created by ChuWen on 9/4/23.
//

#ifndef TCPSERVER_ACCEPTOR_H
#define TCPSERVER_ACCEPTOR_H

#include "P2PSession.h"
#include "P2PSessionFactory.h"
#include "csm-utilities/Thread.h"

namespace csm
{

    namespace service
    {

        class Acceptor
        {
        public:
            explicit Acceptor(std::shared_ptr<P2PSessionFactory> p2pSessionFactory);

        public:
            int init(int fd);

            int start();

            int stop();

            void onConnect();

            void setNewClientCallback(std::function<void(P2PSession::Ptr p2pSession)> newClientCallback);

        private:
            std::shared_ptr<P2PSessionFactory> m_p2pSessionFactory;

            std::mutex x_connect;
            std::condition_variable m_connectCv;

            // 客户端初始化完成回调
            std::function<void(P2PSession::Ptr)> m_newClientCallback;

            std::unique_ptr<utilities::Thread> m_thread;
        };

    } // namespace service

} // namespace csm

#endif // TCPSERVER_ACCEPTOR_H
