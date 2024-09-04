//
// Created by ChuWen on 9/6/23.
//

#ifndef COPYSTATEMACHINE_SESSIONDISPATCHER_H
#define COPYSTATEMACHINE_SESSIONDISPATCHER_H

#include "SlaveReactor.h"

#include <readerwriterqueue.h>

namespace csm
{

    namespace service
    {

        class SessionDispatcher
        {
        public:
            using Ptr = std::shared_ptr<SessionDispatcher>;

            /**
             *
             * @param redispatchInterval    多少个客户端后重新查找管理客户端数量最少的SlaveReactor，下一个客户端将会放到最少的这个SlaveReactor中
             * @param id    本节点ID
             * @param slaveReactors     要管理的子Reactor
             * @return
             */
            SessionDispatcher(const std::size_t redispatchInterval, const std::string& id, const std::size_t slaveReactorSize);
            ~SessionDispatcher();

        public:
            int init();

            int start();

            int stop();

            int addSession(const int fd, std::function<void(const std::size_t slaveReactorIndex)> callback);

            int removeFdSlaveReactorRelation(const int fd);

            int getSlaveReactorIndexByFd(const int fd);

#if 0
            int sendData(const int fd, const std::vector<char> &data);

            void registerClientInfoHandler(std::function<int(const HostEndPointInfo &localHostEndPointInfo,
                                                             const HostEndPointInfo &peerHostEndPointInfo, const int fd, const std::string &id,
                                                             const std::string &uuid)> clientInfoHandler);

            void registerClientInfoReplyHandler(std::function<int(const HostEndPointInfo &hostEndPointInfo,
                                                                  const int fd, const std::string &id, const std::string &uuid, const int result,
                                                                  int &anotherConnectionFd)> clientInfoReplyHandler);

            void registerModuleMessageHandler(std::function<void(const int, const std::int32_t,
                                                                 std::shared_ptr<std::vector<char>> &)> messageHandler);

            void registerDisconnectHandler(
                    std::function<void(const HostEndPointInfo &hostEndPointInfo, const std::string &id,
                                       const std::string &uuid, const int flag)> disconnectHandler);

            // Todo: TcpSessionDestoryer处理fd客户端断开的情况时，需要将SessionDispatcher中的fd也移除
            int disconnectClient(const int fd);
#endif

        private:
            std::size_t m_redispatchInterval;
            std::string m_id;
            std::size_t m_slaveReactorSize;

            // 新上线的客户端队列，等待分发到各个SlaveReactor
            struct SessionInfo
            {
                using Ptr = std::shared_ptr<SessionInfo>;

                SessionInfo(const int f, std::function<void(const std::size_t slaveReactorIndex)> c) : fd(f), callback(std::move(c))
                {}

                int fd;
                std::function<void(const std::size_t slaveReactorIndex)> callback;
            };
            moodycamel::BlockingReaderWriterQueue<SessionInfo::Ptr> m_tcpSessionsQueue;

            // 当前管理最少fd的SlaveReactor index（非实时刷新）
            std::size_t m_slaveReactorIndexWhichHasLeastFd{ 0 };
            // 每个SlaveReactor管理的fd数量
            std::vector<std::unique_ptr<std::atomic_uint32_t>> m_slaveReactorFdSize;

            // clientfd是谁那个SlaveReactor管理的，clientfd=>SlaveReactor index
            std::mutex x_fdSlaveReactorIndex;
            std::unordered_map<int, std::size_t> m_fdSlaveReactorIndex;

            std::atomic_bool m_isTerminate{false};
            utilities::Thread m_thread;
        };

    } // service

} // csm

#endif //COPYSTATEMACHINE_SESSIONDISPATCHER_H
