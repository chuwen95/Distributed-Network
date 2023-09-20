//
// Created by root on 9/6/23.
//

#include "SelectListenner.h"
#include "libcomponents/Logger.h"

namespace components
{

    SelectListenner::SelectListenner()
    {}

    SelectListenner::~SelectListenner()
    {}

    int SelectListenner::init(const int fd)
    {
        m_listenfd = fd;

        return 0;
    }

    int SelectListenner::uninit()
    {
        return 0;
    }

    int SelectListenner::start()
    {
        const auto expression = [this]()
        {
            fd_set readfds;

            // 清空描述符集合
            FD_ZERO(&readfds);

            // 将监听套接字加入到集合中
            FD_SET(m_listenfd, &readfds);

            timeval tm{0, 100 * 1000};
            int ret = select(m_listenfd + 1, &readfds, nullptr, nullptr, &tm);
            if(-1 == ret)
            {
                components::Singleton<components::Logger>::instance()->write(components::LogType::Log_Error, FILE_INFO,"select error, errno: ", errno, ", ",
                                                                             strerror(errno));
                return -1;
            }

            if(FD_ISSET(m_listenfd, &readfds))
            {
                if(nullptr != m_connectHandler)
                {
                    components::Singleton<components::Logger>::instance()->write(components::LogType::Log_Trace, FILE_INFO, "client online");
                    m_connectHandler();
                }
            }

            return 0;
        };
        m_thread.init(expression, 0, "main_reac");
        m_thread.start();

        return 0;
    }

    int SelectListenner::stop()
    {
        m_thread.stop();
        m_thread.uninit();

        return 0;
    }

    void SelectListenner::registerConnectHandler(std::function<void()> connectHandler)
    {
        m_connectHandler = connectHandler;
    }

} // components