//
// Created by root on 9/9/23.
//

#include "ClientAliveChecker.h"
#include "libcomponents/Logger.h"

namespace server
{

    // ClientAliveChecker任务运行间隔，每间隔一段时间运行一次检查
    constexpr std::size_t c_aliveCheckInterval{3000};
    // 若c_aliveTimeout时间内没有收到任何数据，判定客户端掉线
    constexpr std::size_t c_aliveTimeout{30000};

    ClientAliveChecker::ClientAliveChecker()
    {}

    ClientAliveChecker::~ClientAliveChecker()
    {}

    int ClientAliveChecker::init()
    {
        const auto expression = [this]()
        {
            std::unordered_map<int, components::CellTimestamp::Ptr> clientLastRecvTime;
            {
                std::unique_lock<std::mutex> ulock(x_clientLastRecvTime);
                clientLastRecvTime = m_clientLastRecvTime;
            }

            for(auto iter = clientLastRecvTime.begin(); iter != clientLastRecvTime.end(); )
            {
                if(iter->second->getElapsedTimeInMilliSec() > c_aliveTimeout)
                {
                    components::Singleton<components::Logger>::instance()->write(components::LogType::Log_Info, FILE_INFO, "client offline, fd: ", iter->first);
                    m_offlinefds.emplace_back(iter->first);
                    iter = clientLastRecvTime.erase(iter);
                }
                else
                {
                    ++iter;
                }
            }

            {
                std::unique_lock<std::mutex> ulock(x_clientLastRecvTime);
                m_clientLastRecvTime =  clientLastRecvTime;
            }
        };
        m_thread.init(expression, c_aliveCheckInterval, "cli_check");

        return 0;
    }

    int ClientAliveChecker::uninit()
    {
        m_thread.uninit();

        return 0;
    }

    int ClientAliveChecker::start()
    {
        m_thread.start();

        return 0;
    }

    int ClientAliveChecker::stop()
    {
        m_thread.stop();

        return 0;
    }

    int ClientAliveChecker::addClient(const int fd)
    {
        std::unique_lock<std::mutex> ulock(x_clientLastRecvTime);

        components::CellTimestamp::Ptr cellTimestamp = std::make_shared<components::CellTimestamp>();
        cellTimestamp->update();
        m_clientLastRecvTime.emplace(fd, cellTimestamp);

        return 0;
    }

    int ClientAliveChecker::removeClient(const int fd)
    {
        std::unique_lock<std::mutex> ulock(x_clientLastRecvTime);
        m_clientLastRecvTime.erase(fd);

        return 0;
    }

    int ClientAliveChecker::refreshClientLastRecvTime(const int fd)
    {
        std::unique_lock<std::mutex> ulock(x_offlinefds);
        auto iter = m_clientLastRecvTime.find(fd);
        if(m_clientLastRecvTime.end() == iter)
        {
            return -1;
        }
        iter->second->update();
        return 0;
    }

    int ClientAliveChecker::getOfflineClient(std::vector<int> &offlinefds)
    {
        offlinefds.clear();

        std::unique_lock<std::mutex> ulock(x_offlinefds);
        m_offlinefds.swap(offlinefds);

        return 0;
    }

} // server