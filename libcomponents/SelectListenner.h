//
// Created by root on 9/6/23.
//

#ifndef TCPSERVER_SELECTLISTENNER_H
#define TCPSERVER_SELECTLISTENNER_H

#include "libcommon/Common.h"
#include "libcomponents/Thread.h"

namespace components
{

    class SelectListenner
    {
    public:
        SelectListenner();
        ~SelectListenner();

    public:
        int init(const int fd);

        int uninit();

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

        components::Thread m_thread;
    };

} // components

#endif //TCPSERVER_SELECTLISTENNER_H
