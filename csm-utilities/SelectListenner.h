//
// Created by ChuWen on 9/6/23.
//

#ifndef TCPSERVER_SELECTLISTENNER_H
#define TCPSERVER_SELECTLISTENNER_H

#include "csm-common/Common.h"
#include "csm-utilities/Thread.h"

namespace csm
{

    namespace utilities
    {

        class SelectListenner
        {
        public:
            using Ptr = std::shared_ptr<SelectListenner>;

            SelectListenner() = default;
            ~SelectListenner() = default;

        public:
            void setListenFd(const int fd);

            int init();

            int start();

            int stop();

            /**
             * @brief   注册连接事件回调
             *
             * @param connectHandler
             */
            void registerConnectHandler(std::function<void()> connectHandler);

        private:
            int m_listenfd;
            std::function<void()> m_connectHandler;

            Thread m_thread;
        };

    } // components

}

#endif //TCPSERVER_SELECTLISTENNER_H
