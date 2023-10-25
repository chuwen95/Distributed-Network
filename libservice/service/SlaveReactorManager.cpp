//
// Created by root on 9/6/23.
//

#include "SlaveReactorManager.h"
#include "libcomponents/Logger.h"

namespace service
{

    SlaveReactorManager::SlaveReactorManager()
    {}

    SlaveReactorManager::~SlaveReactorManager()
    {}

    int SlaveReactorManager::init(const std::size_t slaveReactorNum, const std::size_t redispatchInterval, const std::string& hostId)
    {
        for(int i = 0; i < slaveReactorNum; ++i)
        {
            SlaveReactor::Ptr slaveReactor = std::make_shared<SlaveReactor>();
            slaveReactor->init(i, hostId);
            m_slaveReactors.emplace_back(slaveReactor);
        }

        const auto expression = [this, redispatchInterval]()
        {
            if(true == m_isTerminate)
            {
                std::this_thread::sleep_for(std::chrono::milliseconds(5));
                return;
            }

            {
                std::unique_lock<std::mutex> ulock(x_tcpSessionsQueue);
                m_tcpSessionsQueueCv.wait(ulock);
            }

            if(true == m_isTerminate)
            {
                std::this_thread::sleep_for(std::chrono::milliseconds(5));
                return;
            }

            // 一定间隔后刷新拥有最少数量的SlaveReactor
            static std::size_t s_refreshTime{0};

            TcpSessionInfo::Ptr tcpSessionInfo{nullptr};
            while(true == m_tcpSessionsQueue.try_dequeue(tcpSessionInfo))
            {
                {
                    // 记录客户端fd所在的SlaveRactor
                    std::unique_lock<std::mutex> ulock(x_clientSlaveReactors);
                    m_clientSlaveReactors[tcpSessionInfo->tcpSession->fd()] = m_slaveReactorIndexWhichHasLeastFd;
                }

                components::Singleton<components::Logger>::instance()->write(components::LogType::Log_Info, FILE_INFO,
                                                                             "dispatch TcpSession to slave reactor, index: ", m_slaveReactorIndexWhichHasLeastFd);
                m_slaveReactors[m_slaveReactorIndexWhichHasLeastFd]->addClient(std::shared_ptr<TcpSession>(tcpSessionInfo->tcpSession));

                ++s_refreshTime;
                if(0 == s_refreshTime % redispatchInterval)
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

                if(nullptr != tcpSessionInfo->callback)
                {
                    tcpSessionInfo->callback();
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
        m_isTerminate = true;
        m_tcpSessionsQueueCv.notify_one();
        m_thread.stop();
        m_thread.uninit();

        for(auto& slaveReactor : m_slaveReactors)
        {
            slaveReactor->stop();
        }

        return 0;
    }

    int SlaveReactorManager::addTcpSession(TcpSession::Ptr tcpSession, std::function<void()> callback)
    {
        TcpSessionInfo::Ptr tcpSessionInfo = std::make_shared<TcpSessionInfo>(tcpSession, callback);

        m_tcpSessionsQueue.enqueue(tcpSessionInfo);
        m_tcpSessionsQueueCv.notify_one();

        return 0;
    }

    std::uint64_t SlaveReactorManager::getClientOnlineTimestamp(const int fd)
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

    int SlaveReactorManager::sendData(const int fd, const std::vector<char> &data)
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

        return slaveReactor->sendData(fd, data.data(), data.size());
    }

    void SlaveReactorManager::registerClientInfoHandler(std::function<int(const HostEndPointInfo& , const HostEndPointInfo&,
            const int, const std::string &, const std::string&)> clientInfoHandler)
    {
        for(auto& slaveReactor : m_slaveReactors)
        {
            slaveReactor->registerClientInfoHandler(clientInfoHandler);
        }
    }

    void SlaveReactorManager::registerClientInfoReplyHandler(std::function<int(const HostEndPointInfo&, const int ,
                                                                                const std::string &, const std::string&, const int, int&)> clientInfoReplyHandler)
    {
        for(auto& slaveReactor : m_slaveReactors)
        {
            slaveReactor->registerClientInfoReplyHandler(clientInfoReplyHandler);
        }
    }

    void SlaveReactorManager::registerModuleMessageHandler(std::function<void(const int, const std::int32_t, std::shared_ptr<std::vector<char>>&)> messageHandler)
    {
        for(auto& slaveReactor : m_slaveReactors)
        {
            slaveReactor->registerModuleMessageHandler(messageHandler);
        }
    }

    void SlaveReactorManager::registerDisconnectHandler(std::function<void(const HostEndPointInfo &hostEndPointInfo,
            const std::string& id, const std::string& uuid, const int flag)> disconnectHandler)
    {
        for(auto& slaveReactor : m_slaveReactors)
        {
            slaveReactor->registerDisconnectHandler([disconnectHandler, this](const int fd, const HostEndPointInfo& hostEndPointInfo,
                    const std::string& id, const std::string& uuid, const int flag){
                {
                    std::unique_lock<std::mutex> ulock(x_clientSlaveReactors);
                    auto iter = m_clientSlaveReactors.find(fd);
                    if (m_clientSlaveReactors.end() == iter)
                    {
                        return 0;
                    }
                    m_clientSlaveReactors.erase(iter);
                }
                disconnectHandler(hostEndPointInfo, id, uuid, flag);
                return 0;
            });
        }
    }

    int SlaveReactorManager::disconnectClient(const int fd)
    {
        std::size_t slaveReactorIndex;

        {
            std::unique_lock<std::mutex> ulock(x_clientSlaveReactors);
            auto iter = m_clientSlaveReactors.find(fd);
            if (m_clientSlaveReactors.end() == iter)
            {
                components::Singleton<components::Logger>::instance()->write(components::LogType::Log_Error, FILE_INFO,
                                                                             "client not exist");
                return -1;
            }
            slaveReactorIndex = iter->second;
        }

        return m_slaveReactors[slaveReactorIndex]->disconnectClient(fd);
    }

} // server