//
// Created by ChuWen on 2024/8/8.
//

#ifndef COPYSTATEMACHINE_SESSIONDESTROYER_H
#define COPYSTATEMACHINE_SESSIONDESTROYER_H

#include "csm-common/Common.h"
#include "csm-utilities/Thread.h"

namespace csm
{

    namespace service
    {

        class SessionDestroyer
        {
        public:
            using Ptr = std::shared_ptr<SessionDestroyer>;

            SessionDestroyer() = default;
            ~SessionDestroyer() = default;

        public:
            int init();

            int start();

            int stop();

            void setHandler(const std::function<int(const int fd, const int flag)> handler);

            int addSession(const int fd, const int flag = 0);

            int addSessions(const std::vector<std::pair<int, int>>& clients);

        private:
            std::function<int(const int fd, const int flag)> m_destoryHandler;

            std::atomic_int m_destroyInterval{ 5 };
            utilities::Thread::Ptr m_thread;

            std::mutex x_waitingDestroySessionInfos;
            std::condition_variable m_waitingDestroySessionInfosCv;
            std::queue<std::pair<int, int>> m_waitingDestroySessionInfos;
        };

    }

}

#endif //COPYSTATEMACHINE_SESSIONDESTROYER_H
