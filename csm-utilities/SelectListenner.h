//
// Created by ChuWen on 9/6/23.
//

#ifndef TCPSERVER_SELECTLISTENNER_H
#define TCPSERVER_SELECTLISTENNER_H

#include "csm-utilities/Thread.h"

namespace csm
{

    namespace utilities
    {

        class SelectListenner
        {
        public:
            void setListenFd(int fd);

            /**
             * @brief   注册连接事件回调
             *
             * @param connectHandler
             */
            void registerConnectHandler(std::function<void()> connectHandler);

            int init();

            int start();

            int stop();

        private:
            int m_listenfd;
            std::function<void()> m_connectHandler;

            std::unique_ptr<utilities::Thread> m_thread;
        };

    } // components

}

#endif //TCPSERVER_SELECTLISTENNER_H
