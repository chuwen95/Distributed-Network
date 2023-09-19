//
// Created by root on 9/6/23.
//

#include "SlaveReactorManager.h"
#include "libcomponents/Logger.h"

namespace server
{

    SlaveReactorManager::SlaveReactorManager()
    {}

    SlaveReactorManager::~SlaveReactorManager()
    {}

    int SlaveReactorManager::init()
    {
        //for(int i = 0; i < std::thread::hardware_concurrency(); ++i)
        for(int i = 0; i < 8; ++i)
        {
            SlaveReactor::Ptr slaveReactor = std::make_shared<SlaveReactor>(i);
            slaveReactor->init();
            m_slaveReactors.emplace_back(slaveReactor);
        }

        const auto expression = [this]()
        {
            {
                std::unique_lock<std::mutex> ulock(x_tcpSessionsQueue);
                m_tcpSessionsQueueCv.wait(ulock);
            }

            // 一定间隔后刷新拥有最少数量的SlaveReactor
            static std::size_t s_refreshTime{0};

            TcpSession::Ptr tcpSession{nullptr};
            while(true == m_tcpSessionsQueue.try_dequeue(tcpSession))
            {
                {
                    // 记录客户端fd所在的SlaveRactor
                    std::unique_lock<std::mutex> ulock(x_clientSlaveReactors);
                    m_clientSlaveReactors[tcpSession->fd()] = m_slaveReactorIndexWhichHasLeastFd;
                }

                components::Singleton<components::Logger>::instance()->write(components::LogType::Log_Info, FILE_INFO,
                                                                             "dispatch TcpSession to slave reactor, index: ", m_slaveReactorIndexWhichHasLeastFd);
                m_slaveReactors[m_slaveReactorIndexWhichHasLeastFd]->addClient(std::shared_ptr<TcpSession>(tcpSession));

                ++s_refreshTime;
                if(0 == s_refreshTime % 1)
                {
                    // 寻找管理最少client fd的SlaveReactor的index
                    std::size_t maxSize = std::numeric_limits<std::size_t>::max();
                    for(int i = 0; i < m_slaveReactors.size(); ++i)
                    {
                        if(maxSize > m_slaveReactors[i]->clientSize())
                        {
                            maxSize = m_slaveReactors[i]->clientSize();
                            m_slaveReactorIndexWhichHasLeastFd = i;
                        }
                    }
                    s_refreshTime = 0;
                }
            }
        };
        m_thread.init(expression, 0, "slave_reac_man");

        return 0;
    }

    int SlaveReactorManager::uninit()
    {
        for(auto& slaveReactor : m_slaveReactors)
        {
            slaveReactor->uninit();
        }
        m_slaveReactors.clear();

        return 0;
    }

    int SlaveReactorManager::start()
    {
        for(auto& slaveReactor : m_slaveReactors)
        {
            slaveReactor->start();
        }

        m_thread.start();

        return 0;
    }

    int SlaveReactorManager::stop()
    {
        m_thread.stop();
        m_thread.uninit();

        for(auto& slaveReactor : m_slaveReactors)
        {
            slaveReactor->stop();
        }

        return 0;
    }

    int SlaveReactorManager::addTcpSession(TcpSession::Ptr tcpSession)
    {
        m_tcpSessionsQueue.enqueue(tcpSession);
        m_tcpSessionsQueueCv.notify_one();

        return 0;
    }

    std::uint32_t SlaveReactorManager::getClientOnlineTimestamp(const int fd)
    {
        SlaveReactor::Ptr slaveReactor;
        {
            std::unique_lock<std::mutex> ulock(x_clientSlaveReactors);

            auto iter = m_clientSlaveReactors.find(fd);
            if(m_clientSlaveReactors.end() == iter)
            {
                return 0;
            }

            slaveReactor = m_slaveReactors[iter->second];
        }

        return slaveReactor->getClientOnlineTimestamp(fd);
    }

    void SlaveReactorManager::registerRecvHandler(std::function<void(const int, const packetprocess::PacketType,
            std::shared_ptr<std::vector<char>>&, std::function<int(const int, const std::vector<char>&)>)> recvHandler)
    {
        for(auto& slaveReactor : m_slaveReactors)
        {
            slaveReactor->registerRecvHandler(recvHandler);
        }
    }

    void SlaveReactorManager::registerDisconnectHandler(std::function<void(const int, const std::string&)> disconnectHandler)
    {
        for(auto& slaveReactor : m_slaveReactors)
        {
            slaveReactor->registerDisconnectHandler([disconnectHandler, this](const int fd, const std::string& id){
                {
                    std::unique_lock<std::mutex> ulock(x_clientSlaveReactors);
                    auto iter = m_clientSlaveReactors.find(fd);
                    if (m_clientSlaveReactors.end() == iter)
                    {
                        return 0;
                    }
                    m_clientSlaveReactors.erase(iter);
                }
                disconnectHandler(fd, id);
                return 0;
            });
        }
    }

} // server