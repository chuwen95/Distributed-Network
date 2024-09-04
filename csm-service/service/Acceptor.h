//
// Created by ChuWen on 9/4/23.
//

#ifndef TCPSERVER_ACCEPTOR_H
#define TCPSERVER_ACCEPTOR_H

#include "csm-common/Common.h"
#include "csm-utilities/Thread.h"
#include "TcpSession.h"

namespace csm
{

    namespace service
    {

        class Acceptor
        {
        public:
            using Ptr = std::shared_ptr<Acceptor>;

            Acceptor() = default;
            ~Acceptor() = default;

        public:
            int init(const int fd);

            int start();

            int stop();

            int onConnect();

            void setNewClientCallback(std::function<void(int, TcpSession::Ptr)> newClientCallback);

        private:
            std::mutex x_connect;
            std::condition_variable m_connectCv;

            // 客户端初始化完成回调
            std::function<void(int, TcpSession::Ptr)> m_newClientCallback;

            std::atomic_bool m_isTerminate{false};
            utilities::Thread m_thread;
        };

    }

}

#endif //TCPSERVER_ACCEPTOR_H
