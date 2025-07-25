//
// Created by ChuWen on 9/25/23.
//

#include "HostsConnector.h"
#include "csm-service/service/P2PSessionFactory.h"
#include "csm-utilities/ElapsedTime.h"
#include "csm-utilities/Logger.h"
#include "csm-utilities/Socket.h"
#include "csm-utilities/TimeTools.h"

#include <json/json.h>

using namespace csm::service;

constexpr std::uint32_t c_connectTimeout{20 * 1000};

HostsConnector::HostsConnector(HostsInfoManager::Ptr hostsInfoManager) : m_hostsInfoManager(std::move(hostsInfoManager)) {}

int HostsConnector::init()
{
    const auto expression = [this]()
    {
        HostsInfoManager::Hosts hosts = m_hostsInfoManager->getHosts();
        for (auto& host : hosts)
        {
            LOG->write(utilities::LogType::Log_Debug, FILE_INFO, "connecting host: ", host.first.host());
            // 如果id不为空，证明客户端已经连上并发送了ClientInfo包
            if (false == host.second.first.empty())
            {
                LOG->write(utilities::LogType::Log_Debug, FILE_INFO, "host is connected: ", host.first.host());
                continue;
            }

            // 如果客户端正在进行连接
            bool isConnecting{false};
            {
                std::unique_lock<std::mutex> ulock(x_connectingHosts);
                isConnecting = (m_connectingHosts.end() != m_connectingHosts.find(host.first));
            }
            if (true == isConnecting)
            {
                LOG->write(utilities::LogType::Log_Debug, FILE_INFO,
                           "host is connecting or waiting ClientInfo payload: ", host.first.ip());
                continue;
            }
            LOG->write(utilities::LogType::Log_Debug, FILE_INFO, "start to connect: ", host.first.host());

            // 开始连接
            int fd = utilities::Socket::create();
            if (-1 == fd)
            {
                LOG->write(utilities::LogType::Log_Error, FILE_INFO, "create socket failed, ready to connect: ", host.first.ip());
                continue;
            }
            LOG->write(utilities::LogType::Log_Debug, FILE_INFO, "create socket successfully, fd: ", fd);

            // 设置socket接收缓冲区大小
            if (-1 == utilities::Socket::setSocketKernelRecvBufferSize(fd, utilities::Socket::c_defaultSocketRecvBufferSize))
            {
                LOG->write(utilities::LogType::Log_Error, FILE_INFO, "set socket recv buffer size failed, errno: ", errno, ", ",
                           strerror(errno));
                return -1;
            }
            LOG->write(utilities::LogType::Log_Debug, FILE_INFO, "set socket recv buffer size to ",
                       utilities::Socket::c_defaultSocketRecvBufferSize, " successfully, fd: ", fd);

            // 添加host到正在连接
            {
                std::unique_lock<std::mutex> ulock(x_connectingHosts);
                m_connectingHosts.emplace(host.first, std::make_pair(fd, utilities::TimeTools::getCurrentTimestamp()));
            }
            LOG->write(utilities::LogType::Log_Debug, FILE_INFO, "add host to connecting successfully, fd: ", fd);

            utilities::Socket::setNonBlock(fd);

            LOG->write(utilities::LogType::Log_Debug, FILE_INFO, "set socket non block successfully, fd: ", fd);

            int ret = utilities::Socket::connect(fd, host.first.ip(), host.first.port());
            if (0 == ret)
            {
                // 一般情况下这个if不会走到，因为非阻塞io第一次connect不会立即返回0，后续又因为m_connectingHosts的原因不会再connect

                LOG->write(utilities::LogType::Log_Debug, FILE_INFO, "connect successfully, host: ", host.first.host(),
                           ", fd: ", fd);

                if (nullptr != m_connectHandler)
                {
                    P2PSession::Ptr p2pSession =
                        P2PSessionFactory().create(fd, service::c_p2pSessionReadBufferSize, service::c_p2pSessionWriteBufferSize);
                    p2pSession->init();
                    p2pSession->setPeerHostEndPointInfo(host.first);

                    // 将P2PSession回调出去，此时仍保留状态为连接中，即不处理m_connectingHosts，等到ClientInfoReply回来后再将fd从m_connectingHosts中移除
                    m_connectHandler(fd, p2pSession);
                }

                // 将时间戳设置为-1，表示已经连上，不需要再监测超时
                {
                    std::unique_lock<std::mutex> ulock(x_connectingHosts);
                    m_connectingHosts[host.first].second = -1;
                }

                continue;
            }
            else if (-1 == ret)
            {
                LOG->write(utilities::LogType::Log_Debug, FILE_INFO, "errno: ", errno);
                if (EINPROGRESS == errno)
                {
                    LOG->write(utilities::LogType::Log_Debug, FILE_INFO, "connect status: EINPROGRESS");
                    continue;
                }
                else
                {
                    LOG->write(utilities::LogType::Log_Error, FILE_INFO, "connect failed");
                    auto iter = m_connectingHosts.find(host.first);
                    if (m_connectingHosts.end() != iter)
                    {
                        m_connectingHosts.erase(iter);
                    }
                    utilities::Socket::close(fd);
                    continue;
                }
            }
        }

        fd_set wset;
        FD_ZERO(&wset);
        int maxfd{0};
        for (auto& host : m_connectingHosts)
        {
            LOG->write(utilities::LogType::Log_Debug, FILE_INFO, "checking ", host.first.host(), " connect status");
            // 将时间戳设置为-1，表示已经连上，不需要再监测超时，
            // P2PSession已经回调出去，由Reactor管理recv/send，此时正在等待ClientInfoReply包
            if (-1 == host.second.second)
            {
                LOG->write(utilities::LogType::Log_Debug, FILE_INFO,
                           "socket connect successfully, waiting ClientInfoReply payload, fd: ", host.second.first);
                continue;
            }

            int fd = host.second.first;
            FD_SET(fd, &wset);
            if (maxfd < fd)
            {
                maxfd = fd;
            }
        }
        timeval tm{0, 500 * 1000};
        int ret = select(maxfd + 1, nullptr, &wset, nullptr, &tm);
        if (-1 == ret)
        {
            LOG->write(utilities::LogType::Log_Error, FILE_INFO, "select error, errno: ", errno, ", ", strerror(errno));
            return -1;
        }
        else if (0 == ret)
        {
        }
        else
        {
            for (auto iter = m_connectingHosts.begin(); iter != m_connectingHosts.end();)
            {
                int fd = iter->second.first;
                if (FD_ISSET(fd, &wset))
                {
                    int error;
                    socklen_t len = sizeof(error);
                    if (-1 == getsockopt(iter->second.first, SOL_SOCKET, SO_ERROR, &error, &len))
                    {
                        utilities::Socket::close(fd);
                        iter = m_connectingHosts.erase(iter);
                        continue;
                    }
                    if (0 != error)
                    {
                        LOG->write(utilities::LogType::Log_Info, FILE_INFO, "connect to ", iter->first.host(),
                                   " failed, error: ", error);
                        utilities::Socket::close(fd);
                        iter = m_connectingHosts.erase(iter);
                        continue;
                    }

                    LOG->write(utilities::LogType::Log_Info, FILE_INFO, "connect to ", iter->first.host(),
                               " successfully, fd is set, fd: ", fd);
                    if (nullptr != m_connectHandler)
                    {
                        LOG->write(utilities::LogType::Log_Info, FILE_INFO, "connect to ", iter->first.host(),
                                   " successfully, callback P2PSession, fd: ", fd);

                        P2PSession::Ptr p2pSession = P2PSessionFactory().create(fd, service::c_p2pSessionReadBufferSize,
                                                                                service::c_p2pSessionWriteBufferSize);
                        p2pSession->init();
                        p2pSession->setPeerHostEndPointInfo(iter->first);

                        // 将P2PSession回调出去，此时仍保留状态为连接中，即不处理m_connectingHosts，等到ClientInfoReply回来后再将fd从m_connectingHosts中移除
                        m_connectHandler(fd, p2pSession);

                        LOG->write(utilities::LogType::Log_Info, FILE_INFO, "connect to ", iter->first.host(),
                                   " callback P2PSession finish, fd: ", fd);
                    }

                    // 将时间戳设置为-1，表示已经连上，不需要再监测超时
                    {
                        std::unique_lock<std::mutex> ulock(x_connectingHosts);
                        m_connectingHosts[iter->first].second = -1;
                    }
                }
                ++iter;
            }
        }

        std::unique_lock<std::mutex> ulock(x_connectingHosts);
        for (auto& host : m_connectingHosts)
        {
            if (-1 == host.second.second)
            {
                continue;
            }

            std::int64_t curTimestamp = utilities::TimeTools::getCurrentTimestamp();
            if (curTimestamp - host.second.second >= c_connectTimeout)
            {
                utilities::Socket::close(host.second.first);
                m_connectingHosts.erase(host.first);
            }
        }

        static int printCircle{0};
        if (0 == printCircle % 6)
        {
            LOG->write(utilities::LogType::Log_Info, FILE_INFO, "online node size: ", m_hostsInfoManager->onlineClientSize());
        }
        printCircle = (printCircle + 1) % 6;

        return 0;
    };
    m_thread = std::make_shared<utilities::Thread>();
    m_thread->setFunc(expression);
    m_thread->setInterval(500);
    m_thread->setName("host_connector");

    return 0;
}

int HostsConnector::start()
{
    m_thread->start();
    return 0;
}

int HostsConnector::stop()
{
    m_thread->stop();
    return 0;
}

int HostsConnector::registerConnectHandler(std::function<void(const int, P2PSession::Ptr)> connectHandler)
{
    m_connectHandler = std::move(connectHandler);

    return 0;
}

int HostsConnector::setHostConnected(const service::HostEndPointInfo& hostEndPointInfo)
{
    std::unique_lock<std::mutex> ulock(x_connectingHosts);
    auto iter = m_connectingHosts.find(hostEndPointInfo);
    if (m_connectingHosts.end() == iter)
    {
        LOG->write(utilities::LogType::Log_Error, FILE_INFO, "host not found");
        return -1;
    }
    m_connectingHosts.erase(iter);

    return 0;
}

int HostsConnector::setHostConnectedByFd(const int fd)
{
    std::unique_lock<std::mutex> ulock(x_connectingHosts);
    for (auto iter = m_connectingHosts.begin(); iter != m_connectingHosts.end(); ++iter)
    {
        if (iter->second.first == fd)
        {
            m_connectingHosts.erase(iter);
            return 0;
        }
    }

    return -1;
}
