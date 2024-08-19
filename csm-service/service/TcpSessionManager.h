//
// Created by ChuWen on 2024/8/8.
//

#ifndef COPYSTATEMACHINE_TCPSESSIONMANAGER_H
#define COPYSTATEMACHINE_TCPSESSIONMANAGER_H

#include "csm-common/Common.h"
#include "TcpSession.h"

namespace csm
{

    namespace service
    {

        class TcpSessionManager
        {
        public:
            using Ptr = std::shared_ptr<TcpSessionManager>;

            TcpSessionManager() = default;
            ~TcpSessionManager() = default;

        public:
            /**
             * 添加TcpSession
             * @param fd
             * @param tcpSession
             * @return
             */
            int addTcpSession(const int fd, TcpSession::Ptr tcpSession);

            /**
             * 移除TcpSession
             * @param fd
             * @return
             */
            int removeTcpSession(const int fd);

            /**
             * TcpSession是否存在
             * @param fd
             * @return
             */
            bool isTcpSessionExist(const int fd);

            /**
             * 获得TcpSession
             * @param fd
             * @return
             */
            TcpSession::Ptr tcpSession(const int fd);

        private:
            std::mutex x_tcpSessions;
            std::unordered_map<int, TcpSession::Ptr> m_tcpSessions;
        };

    }

}

#endif //COPYSTATEMACHINE_TCPSESSIONMANAGER_H
