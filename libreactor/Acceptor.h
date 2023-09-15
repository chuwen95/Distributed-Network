//
// Created by root on 9/4/23.
//

#ifndef TCPSERVER_ACCEPTOR_H
#define TCPSERVER_ACCEPTOR_H

#include "libcommon/Common.h"
#include "libcomponents/Thread.h"
#include "libcomponents/SPSCLockLessQueue.h"
#include "TcpSession.h"

namespace server
{

    class Acceptor
    {
    public:
        using Ptr = std::shared_ptr<Acceptor>;

        Acceptor() = default;
        ~Acceptor() = default;

    public:
        int init(const int fd);

        int uninit();

        int start();

        int stop();

        int onConnect();

        void setNewClientCallback(std::function<void(int, TcpSession*)> newClientCallback);

    private:
        std::mutex x_connect;
        std::condition_variable m_connectCv;

        // 客户端初始化完成回调
        std::function<void(int, TcpSession*)> m_newClientCallback;

        components::Thread m_thread;
    };

}


#endif //TCPSERVER_ACCEPTOR_H
