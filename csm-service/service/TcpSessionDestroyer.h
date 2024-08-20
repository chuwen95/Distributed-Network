//
// Created by ChuWen on 2024/8/8.
//

#ifndef COPYSTATEMACHINE_TCPSESSIONDESTROYER_H
#define COPYSTATEMACHINE_TCPSESSIONDESTROYER_H

#include "csm-common/Common.h"
#include "TcpSessionManager.h"
#include "csm-service/host/HostsConnector.h"
#include "csm-service/host/HostsInfoManager.h"
#include "csm-utilities/Thread.h"

namespace csm
{

    namespace service
    {

        class TcpSessionDestroyer
        {
        public:
            using Ptr = std::shared_ptr<TcpSessionDestroyer>;

            TcpSessionDestroyer(TcpSessionManager::Ptr tcpSessionManager, HostsConnector::Ptr hostsConnector, HostsInfoManager::Ptr hostsInfoManager);
            ~TcpSessionDestroyer() = default;

        public:
            int init();

            int start();

            int stop();

            int addFd(const int fd);

        private:
            TcpSessionManager::Ptr m_tcpSessionManager;
            HostsConnector::Ptr m_hostsConnector;
            HostsInfoManager::Ptr m_hostsInfoManager;

            utilities::Thread m_thread;

            std::mutex x_fds;
            std::queue<int> m_fds;
        };

    }

}

#endif //COPYSTATEMACHINE_TCPSESSIONDESTROYER_H
