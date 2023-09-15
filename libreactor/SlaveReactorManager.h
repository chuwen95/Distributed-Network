//
// Created by root on 9/6/23.
//

#ifndef TCPSERVER_SLAVEREACTORMANAGER_H
#define TCPSERVER_SLAVEREACTORMANAGER_H

#include "SlaveReactor.h"

#include "libcomponents/SPSCLockLessQueue.h"

namespace server
{

    class SlaveReactorManager
    {
    public:
        SlaveReactorManager();
        ~SlaveReactorManager();

        int init();

        int uninit();

        int start();

        int stop();

        int addTcpSession(TcpSession* tcpSession);

        void registerRecvHandler(std::function<void(const int id, const packetprocess::PacketType,
                std::shared_ptr<std::vector<char>>&, std::function<int(const int, const std::vector<char>&)>)> recvHandler);

        void registerDisconnectHandler(std::function<void(const int id)> disconnectHandler);

        int sendData(const int fd, const char* data, const std::size_t size);

    public:
        // 新上线客户端的信号
        std::mutex x_tcpSessionsQueue;
        std::condition_variable m_tcpSessionsQueueCv;

        // 新上线的客户端队列，等待分发到各个SlaveReactor
        components::spsc_queue_t<TcpSession*> m_tcpSessionsQueue;

        // 所有的子Reactor集合
        std::vector<SlaveReactor::Ptr> m_slaveReactors;

        // 管理最少fd的SlaveReactor
        std::atomic_int m_slaveReactorIndexWhichHasLeastFd{0};

        // clientfd是谁那个SlaveReactor管理的，clientfd=>SlaveReactor index
        std::mutex x_clientSlaveReactors;
        std::unordered_map<int, std::size_t> m_clientSlaveReactors;

        components::Thread m_thread;
    };

} // server

#endif //TCPSERVER_SLAVEREACTORMANAGER_H
