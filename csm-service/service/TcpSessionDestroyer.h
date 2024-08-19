//
// Created by ChuWen on 2024/8/8.
//

#ifndef COPYSTATEMACHINE_TCPSESSIONDESTROYER_H
#define COPYSTATEMACHINE_TCPSESSIONDESTROYER_H

#include "csm-common/Common.h"
#include "TcpSessionManager.h"
#include "csm-utilities/Thread.h"

namespace csm
{

    namespace service
    {

        class TcpSessionDestroyer
        {
        public:
            using Ptr = std::shared_ptr<TcpSessionDestroyer>;

            TcpSessionDestroyer() = default;
            ~TcpSessionDestroyer() = default;

        public:
            int init();

            int start();

            int stop();

            int addFd(const int fd);

        private:
            utilities::Thread m_thread;
        };

    }

}

#endif //COPYSTATEMACHINE_TCPSESSIONDESTROYER_H
