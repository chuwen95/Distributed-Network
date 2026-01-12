//
// Created by ChuWen on 2024/8/8.
//

#ifndef COPYSTATEMACHINE_SESSIONDESTROYER_H
#define COPYSTATEMACHINE_SESSIONDESTROYER_H

#include <queue>

#include "P2PSession.h"
#include "csm-utilities/Thread.h"

namespace csm
{

    namespace service
    {

        struct SessionDestoryInfo
        {
            SessionDestoryInfo() = default;

            explicit SessionDestoryInfo(SessionId s, P2PSession::WPtr p, int f = 0, bool i = false) :
                sessionId(s), p2pSessionWeakPtr(p), flag(f), isFromAliveChecker(i)
            {
            }

            SessionId sessionId;
            P2PSession::WPtr p2pSessionWeakPtr;
            int flag;
            bool isFromAliveChecker;
        };

        class SessionDestroyer
        {
        public:
            using Ptr = std::shared_ptr<SessionDestroyer>;

        public:
            int init();

            int start();

            int stop();

            void setHandler(std::function<void(const SessionDestoryInfo& info)> handler);

            void addSession(const SessionDestoryInfo& session);

            void addSessions(const std::vector<SessionDestoryInfo>& sessions);

        private:
            std::function<void(const SessionDestoryInfo& info)> m_destoryHandler;

            std::atomic_int m_destroyInterval{1};
            std::unique_ptr<utilities::Thread> m_thread;

            std::mutex x_waitingDestroySessionInfos;
            std::condition_variable m_waitingDestroySessionInfosCv;

            std::queue<SessionDestoryInfo> m_waitingDestroySessionInfos;
        };

    } // namespace service

} // namespace csm

#endif // COPYSTATEMACHINE_SESSIONDESTROYER_H