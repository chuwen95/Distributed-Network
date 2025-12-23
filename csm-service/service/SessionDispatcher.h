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
            /**
             *
             * @param redispatchInterval
             * 多少个客户端后重新查找管理客户端数量最少的SlaveReactor，下一个客户端将会放到最少的这个SlaveReactor中
             * @param id    本节点ID
             * @param slaveReactors     要管理的子Reactor
             * @return
             */
            explicit SessionDispatcher(std::size_t slaveReactorSize);
            ~SessionDispatcher() = default;

        public:
            int init();

            int start();

            int stop();

            int addSession(P2PSession::Ptr p2pSession, std::function<void(std::size_t slaveReactorIndex)> callback);

            int removeSessionIdSlaveReactorRelation(SessionId sessionId);

            int getSlaveReactorIndexBySessionId(SessionId sessionId);

        private:
            std::size_t m_slaveReactorSize;
            std::vector<std::size_t> m_slaveReactorSessionSize;

            // 新上线的客户端队列，等待分发到各个SlaveReactor
            struct SessionInfo
            {
                using Ptr = std::shared_ptr<SessionInfo>;

                SessionInfo(P2PSession::Ptr p, std::function<void(const std::size_t slaveReactorIndex)> c)
                    : p2pSession(std::move(p)), callback(std::move(c))
                {
                }

                P2PSession::Ptr p2pSession;
                std::function<void(std::size_t slaveReactorIndex)> callback;
            };
            moodycamel::BlockingReaderWriterQueue<SessionInfo::Ptr> m_p2pSessionsQueue;

            // sessionId是谁那个SlaveReactor管理的，sessionId=>SlaveReactor index
            std::mutex x_sessionIdSlaveReactorIndex;
            std::unordered_map<SessionId, std::size_t> m_sessionIdSlaveReactorIndex;

            std::atomic_bool m_isTerminate{false};
            std::unique_ptr<utilities::Thread> m_thread;
        };

    } // service

} // csm

#endif //COPYSTATEMACHINE_SESSIONDISPATCHER_H
