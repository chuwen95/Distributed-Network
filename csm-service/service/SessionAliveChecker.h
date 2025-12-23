//
// Created by ChuWen on 9/9/23.
//

#ifndef SESSIONALIVECHECKER_H
#define SESSIONALIVECHECKER_H

#include <google/protobuf/message.h>

#include "csm-common/Common.h"
#include "csm-utilities/Thread.h"
#include "P2PSession.h"

namespace csm
{

    namespace service
    {

        class SessionAliveChecker
        {
        public:
            int init();

            int start();

            int stop();

            void setTimeoutHandler(std::function<void(const std::vector<std::pair<SessionId, P2PSession::WPtr>> &)> handler);

            /**
             * @brief 添加要监测是否在线的客户端
             *
             * @param sessionId
             * @return
             */
            int addSession(SessionId sessionId, P2PSession::WPtr p2pSessionWeakPtr);

            /**
             * @brief 移除要监测是否在线的客户端
             *
             * @param sessionId
             * @return
             */
            int removeSession(SessionId sessionId);

        private:
            std::mutex x_sessions;
            std::unordered_map<std::uint64_t, P2PSession::WPtr> m_sessions;

            std::unique_ptr<utilities::Thread> m_thread;
            std::function<void(const std::vector<std::pair<SessionId, P2PSession::WPtr>>&)> m_timeoutHandler;
        };

    } // server

}

#endif //SESSIONALIVECHECKER_H