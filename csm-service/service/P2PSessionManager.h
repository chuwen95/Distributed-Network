//
// Created by ChuWen on 2024/8/8.
//

#ifndef COPYSTATEMACHINE_P2PSESSIONMANAGER_H
#define COPYSTATEMACHINE_P2PSESSIONMANAGER_H

#include "P2PSession.h"
#include "csm-common/Common.h"

namespace csm
{

    namespace service
    {

        class P2PSessionManager
        {
        public:
            using Ptr = std::shared_ptr<P2PSessionManager>;

            P2PSessionManager() = default;
            ~P2PSessionManager() = default;

        public:
            /**
             * 添加Session
             * @param fd
             * @param p2pSession
             * @return
             */
            int addSession(const int fd, P2PSession::Ptr p2pSession);

            /**
             * 移除Session
             * @param fd
             * @return
             */
            int removeSession(const int fd);

            /**
             * Session是否存在
             * @param fd
             * @return
             */
            bool isSessionExist(const int fd);

            /**
             * 获得Session
             * @param fd
             * @return
             */
            P2PSession::Ptr session(const int fd);

            /*
             * 获取管理的P2PSessions数量
             */
            std::size_t sessionSize();

        private:
            std::mutex x_sessions;
            std::unordered_map<int, P2PSession::Ptr> m_sessions;
        };

    } // namespace service

} // namespace csm

#endif // COPYSTATEMACHINE_P2PSESSIONMANAGER_H
