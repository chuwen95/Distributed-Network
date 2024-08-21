//
// Created by ChuWen on 9/6/23.
//

#include "SessionDispatcher.h"
#include "csm-utilities/Logger.h"

using namespace csm::service;

SessionDispatcher::SessionDispatcher(const std::size_t redispatchInterval, const std::string &hostId, const std::size_t slaveReactorSize) :
        m_redispatchInterval(redispatchInterval), m_id(hostId), m_slaveReactorSize(slaveReactorSize)
{
    for(std::size_t i = 0; i < m_slaveReactorSize; ++i)
    {
        m_slaveReactorFdSize.emplace_back(std::make_unique<std::atomic_uint32_t>());
    }
}

SessionDispatcher::~SessionDispatcher()
{}

int SessionDispatcher::init()
{
    const auto expression = [this]() {
        {
            std::unique_lock<std::mutex> ulock(x_tcpSessionsQueue);
            m_tcpSessionsQueueCv.wait(ulock);
        }

        if (true == m_isTerminate)
        {
            return;
        }

        // 一定间隔后刷新拥有最少数量的SlaveReactor
        static std::size_t s_refreshTime{0};

        SessionInfo::Ptr sessionInfo{nullptr};
        while (true == m_tcpSessionsQueue.try_dequeue(sessionInfo))
        {
            {
                // 记录客户端fd所在的SlaveRactor
                std::unique_lock<std::mutex> ulock(x_fdSlaveReactorIndex);
                m_fdSlaveReactorIndex[sessionInfo->fd] = m_slaveReactorIndexWhichHasLeastFd;
            }

            LOG->write(utilities::LogType::Log_Info, FILE_INFO, "dispatch TcpSession to slave reactor, index: ", m_slaveReactorIndexWhichHasLeastFd);
            sessionInfo->callback(m_slaveReactorIndexWhichHasLeastFd);

            ++s_refreshTime;
            if (0 == s_refreshTime % m_redispatchInterval)
            {
                // 寻找管理最少client fd的SlaveReactor的index
                std::size_t maxSize = std::numeric_limits<std::size_t>::max();
                for (std::size_t i = 0; i < m_slaveReactorSize; ++i)
                {
                    if (maxSize > *(m_slaveReactorFdSize[i]))
                    {
                        maxSize = *(m_slaveReactorFdSize[i]);
                        m_slaveReactorIndexWhichHasLeastFd = i;
                    }
                }
                s_refreshTime = 0;
            }
        }
    };
    m_thread.init(expression, 1, "tcp_session_dis");

    return 0;
}

int SessionDispatcher::uninit()
{
    return 0;
}

int SessionDispatcher::start()
{
    m_thread.start();
    LOG->write(utilities::LogType::Log_Info, FILE_INFO, "start slave reactor manager successfully");

    return 0;
}

int SessionDispatcher::stop()
{
    m_isTerminate = true;
    m_tcpSessionsQueueCv.notify_one();
    m_thread.stop();
    m_thread.uninit();

    return 0;
}

int SessionDispatcher::addSession(const int fd, std::function<void(const std::size_t slaveReactorIndex)> callback)
{
    SessionInfo::Ptr tcpSessionInfo = std::make_shared<SessionInfo>(fd, callback);

    m_tcpSessionsQueue.enqueue(tcpSessionInfo);
    m_tcpSessionsQueueCv.notify_one();

    return 0;
}

int SessionDispatcher::removeFdSlaveReactorRelation(const int fd)
{
    std::unique_lock<std::mutex> ulock(x_fdSlaveReactorIndex);

    auto iter = m_fdSlaveReactorIndex.find(fd);
    if (m_fdSlaveReactorIndex.end() == iter)
    {
        return -1;
    }

    m_fdSlaveReactorIndex.erase(fd);

    return 0;
}

int SessionDispatcher::getSlaveReactorIndexByFd(const int fd)
{
    std::unique_lock<std::mutex> ulock(x_fdSlaveReactorIndex);

    auto iter = m_fdSlaveReactorIndex.find(fd);
    if (m_fdSlaveReactorIndex.end() == iter)
    {
        return -1;
    }

    return iter->second;
}

int SessionDispatcher::sendData(const int fd, const std::vector<char> &data)
{
    SlaveReactor::Ptr slaveReactor;
    {
        std::unique_lock<std::mutex> ulock(x_clientSlaveReactors);

        auto iter = m_clientSlaveReactors.find(fd);
        if (m_clientSlaveReactors.end() == iter)
        {
            return 0;
        }

        slaveReactor = m_slaveReactors[iter->second];
    }

    return slaveReactor->sendData(fd, data.data(), data.size());
}

void SessionDispatcher::registerClientInfoHandler(std::function<int(const HostEndPointInfo &, const HostEndPointInfo &,
                                                                    const int, const std::string &, const std::string &)> clientInfoHandler)
{
    for (auto &slaveReactor: m_slaveReactors)
    {
        slaveReactor->registerClientInfoHandler(clientInfoHandler);
    }
}

void SessionDispatcher::registerClientInfoReplyHandler(std::function<int(const HostEndPointInfo &, const int,
                                                                         const std::string &, const std::string &, const int, int &)> clientInfoReplyHandler)
{
    for (auto &slaveReactor: m_slaveReactors)
    {
        slaveReactor->registerClientInfoReplyHandler(clientInfoReplyHandler);
    }
}

void SessionDispatcher::registerModuleMessageHandler(
        std::function<void(const int, const std::int32_t, std::shared_ptr<std::vector<char>> &)> messageHandler)
{
    for (auto &slaveReactor: m_slaveReactors)
    {
        slaveReactor->registerModuleMessageHandler(messageHandler);
    }
}

void SessionDispatcher::registerDisconnectHandler(std::function<void(const HostEndPointInfo &hostEndPointInfo,
                                                                     const std::string &id, const std::string &uuid, const int flag)> disconnectHandler)
{
    for (auto &slaveReactor: m_slaveReactors)
    {
        slaveReactor->registerDisconnectHandler(
                [disconnectHandler, this](const int fd, const HostEndPointInfo &hostEndPointInfo,
                                          const std::string &id, const std::string &uuid, const int flag) {
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

int SessionDispatcher::disconnectClient(const int fd)
{
    std::size_t slaveReactorIndex;

    {
        std::unique_lock<std::mutex> ulock(x_clientSlaveReactors);
        auto iter = m_clientSlaveReactors.find(fd);
        if (m_clientSlaveReactors.end() == iter)
        {
            LOG->write(utilities::LogType::Log_Error, FILE_INFO,
                       "client not exist");
            return -1;
        }
        slaveReactorIndex = iter->second;
    }

    return m_slaveReactors[slaveReactorIndex]->disconnectClient(fd);
}