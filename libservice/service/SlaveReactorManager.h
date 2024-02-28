//
// Created by root on 9/6/23.
//

#ifndef TCPSERVER_SLAVEREACTORMANAGER_H
#define TCPSERVER_SLAVEREACTORMANAGER_H

#include "SlaveReactor.h"

#include <readerwriterqueue.h>

namespace service
{

    class SlaveReactorManager
    {
    public:
        using Ptr = std::shared_ptr<SlaveReactorManager>;

        SlaveReactorManager();
        ~SlaveReactorManager();

        void addSlaveReactor(SlaveReactor::Ptr slaveReactor);

        /**
         *
         * @param redispatchInterval    多少个客户端后重新查找管理客户端数量最少的SlaveReactor，下一个客户端将会放到最少的这个SlaveReactor中
         * @return
         */
        int init(const std::size_t redispatchInterval, const std::string& id);

        int uninit();

        int start();

        int stop();

        int addTcpSession(TcpSession::Ptr tcpSession, std::function<void()> callback = nullptr);

        std::uint64_t getClientOnlineTimestamp(const int fd);

        int sendData(const int fd, const std::vector<char>& data);

        void registerClientInfoHandler(std::function<int(const HostEndPointInfo& localHostEndPointInfo,
                                                         const HostEndPointInfo& peerHostEndPointInfo, const int fd, const std::string& id, const std::string& uuid)> clientInfoHandler);

        void registerClientInfoReplyHandler(std::function<int(const HostEndPointInfo& hostEndPointInfo,
                                                              const int fd, const std::string& id, const std::string& uuid, const int result, int& anotherConnectionFd)> clientInfoReplyHandler);

        void registerModuleMessageHandler(std::function<void(const int, const std::int32_t, std::shared_ptr<std::vector<char>>&)> messageHandler);

        void registerDisconnectHandler(std::function<void(const HostEndPointInfo &hostEndPointInfo, const std::string& id,
                                                          const std::string& uuid, const int flag)> disconnectHandler);

        int disconnectClient(const int fd);

    private:
        // 新上线客户端的信号
        std::mutex x_tcpSessionsQueue;
        std::condition_variable m_tcpSessionsQueueCv;

        // 新上线的客户端队列，等待分发到各个SlaveReactor
        struct TcpSessionInfo
        {
            using Ptr = std::shared_ptr<TcpSessionInfo>;
            TcpSessionInfo(TcpSession::Ptr t, std::function<void()> c) : tcpSession(t), callback(c) {}

            TcpSession::Ptr tcpSession;
            std::function<void()> callback;
        };
        moodycamel::ReaderWriterQueue<TcpSessionInfo::Ptr> m_tcpSessionsQueue;

        // 所有的子Reactor集合
        std::vector<SlaveReactor::Ptr> m_slaveReactors;

        // 管理最少fd的SlaveReactor
        std::size_t m_slaveReactorIndexWhichHasLeastFd{0};

        // clientfd是谁那个SlaveReactor管理的，clientfd=>SlaveReactor index
        std::mutex x_clientSlaveReactors;
        std::unordered_map<int, std::size_t> m_clientSlaveReactors;

        std::atomic_bool m_isTerminate{false};
        components::Thread m_thread;
    };

} // server

#endif //TCPSERVER_SLAVEREACTORMANAGER_H
