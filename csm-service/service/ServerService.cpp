//
// Created by ChuWen on 9/5/23.
//

#include "ServerService.h"

#include "csm-utilities/Logger.h"
#include "csm-utilities/Socket.h"
#include "csm-service/protocol/payload/PayloadHeartBeat.h"
#include "csm-service/protocol/payload/PayloadHeartBeatReply.h"
#include "csm-service/protocol/utilities/PacketEncodeHelper.h"

#include <uuid.h>

using namespace csm::service;

constexpr std::size_t c_conQueueSize{500};

ServerService::ServerService(std::unique_ptr<ServerServiceConfig> serviceConfig) : m_serviceConfig(std::move(serviceConfig))
{
}

ServerService::~ServerService()
{
    // 关闭套接字
    utilities::Socket::close(m_fd);
}

int ServerService::init()
{
    LOG->write(utilities::LogType::Log_Info, FILE_INFO,
               "****************************************TcpServer::init****************************************");

    if (-1 == m_serviceConfig->sessionDispatcher()->init())
    {
        LOG->write(utilities::LogType::Log_Error, FILE_INFO, "session dispatcher init failed");
        return -1;
    }
    LOG->write(utilities::LogType::Log_Info, FILE_INFO, "session dispatcher init successfully");

    if (-1 == m_serviceConfig->sessionDestroyer()->init())
    {
        LOG->write(utilities::LogType::Log_Error, FILE_INFO, "session destroyer init failed");
        return -1;
    }
    LOG->write(utilities::LogType::Log_Info, FILE_INFO, "session destroyer init successfully");

    // 当flag为0的时候，是因为socket发生错误而断开，如果是-1/-2/-3/-4则是握手协议ClientInfoReply包的返回码
    m_serviceConfig->sessionDestroyer()->setHandler(
        [this](const SessionDestoryInfo& destoryInfo) {
            LOG->write(utilities::LogType::Log_Info, FILE_INFO, "start destory tcp session, session id: ", destoryInfo.sessionId,
                       ", flag: ",
                       destoryInfo.flag);

            P2PSession::Ptr p2pSession = destoryInfo.p2pSessionWeakPtr.lock();
            if (nullptr == p2pSession)
            {
                LOG->write(utilities::LogType::Log_Info, FILE_INFO, "p2p session may be already destoryed, session id: ",
                           p2pSession->sessionId());
                return;
            }

            LOG->write(utilities::LogType::Log_Info, FILE_INFO, "set tcp session status to be waiting disconnect, session id: ",
                       p2pSession->sessionId());
            p2pSession->setWaitingDisconnect(true);

            // 不再检查心跳
            LOG->write(utilities::LogType::Log_Info, FILE_INFO, "is from alive checker: ", destoryInfo.isFromAliveChecker,
                       ", session id: ", p2pSession->sessionId());
            if (false == destoryInfo.isFromAliveChecker)
            {
                LOG->write(utilities::LogType::Log_Info, FILE_INFO, "remove tcp session from client alive checker, session id: ",
                           p2pSession->sessionId());
                m_serviceConfig->sessionAliveChecker()->removeSession(p2pSession->sessionId());
            }

            // 将fd从子Reactor中移除
            LOG->write(utilities::LogType::Log_Info, FILE_INFO, "remove fd from slave reactor, fd: ",
                       p2pSession->fd());
            SlaveReactor* slaveReactor = m_serviceConfig->slaveReactorPool()->slaveReactor(p2pSession->slaveReactorIndex());
            if (-1 == slaveReactor->removeClient(p2pSession->sessionId(), p2pSession->fd()))
            {
                LOG->write(utilities::LogType::Log_Error, FILE_INFO, "remove fd from slave reactor epoll failed, session id: ",
                           p2pSession->sessionId());
            }

            LOG->write(utilities::LogType::Log_Info, FILE_INFO, "remove fd slave reactor relation, session id: ",
                       p2pSession->sessionId());
            m_serviceConfig->sessionDispatcher()->removeSessionIdSlaveReactorRelation(p2pSession->sessionId());

            utilities::Socket::close(p2pSession->fd());
        });

    for (SlaveReactor* slaveReactor : m_serviceConfig->slaveReactorPool()->slaveReactors())
    {
        if (-1 == slaveReactor->init())
        {
            LOG->write(utilities::LogType::Log_Error, FILE_INFO, "slave reactor init failed");
            return -1;
        }
        LOG->write(utilities::LogType::Log_Info, FILE_INFO, "slave reactor init successfully");

        slaveReactor->setDisconnectHandler([this](const SessionId sessionId, P2PSession::WPtr p2pSessionWeakPtr) {
            m_serviceConfig->sessionDestroyer()->
                             addSession(SessionDestoryInfo(sessionId, std::move(p2pSessionWeakPtr), 0, false));
        });
        slaveReactor->setSessionDataHandler(
            [this](const SessionId sessionId, P2PSession::WPtr p2pSessionWeakPtr, const char* data, const std::size_t dataLen) {
                m_serviceConfig->sessionDataDecoder()->addSessionData(sessionId, std::move(p2pSessionWeakPtr), data, dataLen);
            });
    }

    m_serviceConfig->sessionAliveChecker()->setTimeoutHandler(
        [this](const std::vector<std::pair<SessionId, P2PSession::WPtr>>& sessions) {
            for (const auto& session : sessions)
            {
                LOG->write(utilities::LogType::Log_Info, FILE_INFO,
                           "heart beat timeout, client offline, add to session destoryer, session id: ", session.first);
                m_serviceConfig->sessionDestroyer()->addSessions(
                    {SessionDestoryInfo(session.first, session.second, 0, true)});
            }
        });
    m_serviceConfig->sessionAliveChecker()->init();

    // 注册会话数据处理回调
    m_serviceConfig->sessionDataDecoder()->setPacketHandler(
        [this](SessionId sessionId, P2PSession::WPtr p2pSessionWeakPtr, PacketHeader::Ptr header,
               PayloadBase::Ptr payload) -> int {
            if (PacketType::PT_HeartBeat == header->type())
            {
                m_serviceConfig->sessionServiceDataProcessor()->addPacket(sessionId, std::move(p2pSessionWeakPtr),
                                                                          std::move(header), std::move(payload));
            }
            else
            {
                if (nullptr != m_packetHandler)
                {
                    m_packetHandler(sessionId, p2pSessionWeakPtr, header, payload);
                }
            }

            return 0;
        });

    // 注册网络模块数据接收回调
    if (0 != m_serviceConfig->sessionServiceDataProcessor()->init())
    {
        LOG->write(utilities::LogType::Log_Error, FILE_INFO, "session service data processor init failed");
        return -1;
    }
    LOG->write(utilities::LogType::Log_Info, FILE_INFO, "session service data processor init successfully");

    if (-1 == initServer())
    {
        LOG->write(utilities::LogType::Log_Error, FILE_INFO, "init server failed");
        return -1;
    }
    LOG->write(utilities::LogType::Log_Info, FILE_INFO, "init server successfully");

    LOG->write(utilities::LogType::Log_Info, FILE_INFO,
               "****************************************TcpServer::init****************************************");

    return 0;
}

int ServerService::start()
{
    LOG->write(utilities::LogType::Log_Info, FILE_INFO,
               "****************************************TcpServer::start****************************************");

    // 包解析器
    m_serviceConfig->sessionDataDecoder()->start();
    // 网络模块包处理器
    m_serviceConfig->sessionServiceDataProcessor()->start();
    // 其它模组包处理器
    m_serviceConfig->sessionModuleDataProcessor()->start();
    LOG->write(utilities::LogType::Log_Info, FILE_INFO, "start session data processor successfully");
    std::vector<SlaveReactor*> slaveReactors = m_serviceConfig->slaveReactorPool()->slaveReactors();
    for (SlaveReactor* slaveReactor : slaveReactors)
    {
        slaveReactor->start();
    }
    LOG->write(utilities::LogType::Log_Info, FILE_INFO, "start all slave reactor successfully");
    // 心跳检查器
    m_serviceConfig->sessionAliveChecker()->start();
    LOG->write(utilities::LogType::Log_Info, FILE_INFO, "start client alive checker successfully");
    // Session派发器
    m_serviceConfig->sessionDispatcher()->start();
    LOG->write(utilities::LogType::Log_Info, FILE_INFO, "start session dispatcher successfully");
    // Session销毁器
    m_serviceConfig->sessionDestroyer()->start();
    LOG->write(utilities::LogType::Log_Info, FILE_INFO, "start session destroyer successfully");
    // 启动主reactor的acceptor
    m_serviceConfig->acceptor()->start();
    LOG->write(utilities::LogType::Log_Info, FILE_INFO, "start acceptor successfully");
    m_serviceConfig->listenner()->start();
    LOG->write(utilities::LogType::Log_Info, FILE_INFO, "start listenner successfully");

    LOG->write(utilities::LogType::Log_Info, FILE_INFO,
               "****************************************TcpServer::start****************************************");

    return 0;
}

int ServerService::stop()
{
    LOG->write(utilities::LogType::Log_Info, FILE_INFO,
               "****************************************TcpServer::stop****************************************");

    m_serviceConfig->sessionAliveChecker()->stop();
    LOG->write(utilities::LogType::Log_Info, FILE_INFO, "stop client heartbeat checker successfully");

    m_serviceConfig->sessionDataDecoder()->stop();
    LOG->write(utilities::LogType::Log_Info, FILE_INFO, "stop session data decoder successfully");
    m_serviceConfig->sessionServiceDataProcessor()->stop();
    LOG->write(utilities::LogType::Log_Info, FILE_INFO, "stop service data processor successfully");
    m_serviceConfig->sessionModuleDataProcessor()->stop();
    LOG->write(utilities::LogType::Log_Info, FILE_INFO, "stop module data processor successfully");
    m_serviceConfig->listenner()->stop();
    LOG->write(utilities::LogType::Log_Info, FILE_INFO, "stop listenner successfully");
    m_serviceConfig->acceptor()->stop();
    LOG->write(utilities::LogType::Log_Info, FILE_INFO, "stop acceptor successfully");
    m_serviceConfig->sessionDestroyer()->stop();
    LOG->write(utilities::LogType::Log_Info, FILE_INFO, "stop session destroyer successfully");
    m_serviceConfig->sessionDispatcher()->stop();
    LOG->write(utilities::LogType::Log_Info, FILE_INFO, "stop session dispatcher successfully");
    std::vector<SlaveReactor*> slaveReactors = m_serviceConfig->slaveReactorPool()->slaveReactors();
    for (SlaveReactor* slaveReactor : slaveReactors)
    {
        slaveReactor->stop();
    }
    LOG->write(utilities::LogType::Log_Info, FILE_INFO, "stop all slave reactor successfully");

    LOG->write(utilities::LogType::Log_Info, FILE_INFO,
               "****************************************TcpServer::stop****************************************");

    return 0;
}

void ServerService::registerPacketHandler(std::function<void(SessionId sessionId, P2PSession::WPtr p2pSessionWeakPtr,
                                                             PacketHeader::Ptr header, PayloadBase::Ptr payload)> handler)
{
    m_packetHandler = std::move(handler);
}

void ServerService::setDisconnectHandler(std::function<void(const HostEndPointInfo& hostEndPointInfo, const NodeId& nodeId,
                                                            int flag, const SessionId& sessionId)> handler)
{
    m_disconnectHandler = std::move(handler);
}

int ServerService::initServer()
{
    // 创建监听套接字
    m_fd = utilities::Socket::create();
    if (-1 == m_fd)
    {
        LOG->write(utilities::LogType::Log_Error, FILE_INFO, "create socket failed, errno: ", errno, ", ", strerror(errno));
        return -1;
    }
    LOG->write(utilities::LogType::Log_Info, FILE_INFO, "create socket successfully, fd: ", m_fd);

#if 0
    if (-1 == utilities::Socket::setNonBlock(m_fd))
    {
        return -1;
    }
#endif

    if (-1 == utilities::Socket::setReuseAddr(m_fd))
    {
        LOG->write(utilities::LogType::Log_Error, FILE_INFO, "set socket reuse addr failed, errno: ", errno, ", ",
                   strerror(errno));
        utilities::Socket::close(m_fd);
        return -1;
    }
    LOG->write(utilities::LogType::Log_Info, FILE_INFO, "set socket reuse addr successfully");

    // 绑定监听地址端口
    std::string listenIp = m_serviceConfig->nodeConfig()->p2pIp();
    unsigned short listenPort = m_serviceConfig->nodeConfig()->p2pPort();
    if (-1 == utilities::Socket::bind(m_fd, listenIp, listenPort))
    {
        LOG->write(utilities::LogType::Log_Error, FILE_INFO, "bind socket failed, errno: ", errno, ", ", strerror(errno),
                   ", listenIp: ", listenIp, ", listenPort:", listenPort);
        utilities::Socket::close(m_fd);
        return -1;
    }
    LOG->write(utilities::LogType::Log_Info, FILE_INFO, "bind socket successfully", ", listenIp: ", listenIp,
               ", listenPort:", listenPort);

    // 设置套接字为被动监听及连接队列长度
    if (-1 == utilities::Socket::listen(m_fd, c_conQueueSize))
    {
        LOG->write(utilities::LogType::Log_Error, FILE_INFO, "listen socket failed, errno: ", errno, ", ", strerror(errno));
        utilities::Socket::close(m_fd);
        return -1;
    }
    LOG->write(utilities::LogType::Log_Info, FILE_INFO, "listen socket successfully");

    if (-1 == m_serviceConfig->acceptor()->init(m_fd))
    {
        LOG->write(utilities::LogType::Log_Error, FILE_INFO, "acceptor init failed");
        utilities::Socket::close(m_fd);
        return -1;
    }
    LOG->write(utilities::LogType::Log_Info, FILE_INFO, "acceptor init successfully");

    m_serviceConfig->listenner()->setListenFd(m_fd);
    if (-1 == m_serviceConfig->listenner()->init())
    {
        LOG->write(utilities::LogType::Log_Error, FILE_INFO, "select listenner init failed");
        utilities::Socket::close(m_fd);
        return -1;
    }
    LOG->write(utilities::LogType::Log_Info, FILE_INFO, "select listenner init successfully");

    // 完成一个客户端的创建，将客户端分配到从reactor进行recv/send处理
    m_serviceConfig->acceptor()->setNewClientCallback(
        [this](P2PSession::Ptr p2pSession) {
            // 将fd添加到P2PSession派发器中，由P2PSessionManager决定P2PSession由哪个SlaveReactor管理
            m_serviceConfig->sessionDispatcher()->addSession(
                std::move(p2pSession),
                [this, p2pSession](const std::size_t slaveReactorIndex) {
                    LOG->write(utilities::LogType::Log_Info, FILE_INFO,
                               "will dispatch P2PSession to SlaveReactor index: ", slaveReactorIndex, ", session id: ",
                               p2pSession->sessionId());

                    SlaveReactor* slaveReactor = m_serviceConfig->slaveReactorPool()->slaveReactor(slaveReactorIndex);
                    assert(nullptr != slaveReactor);

                    if (-1 == slaveReactor->addSession(p2pSession->fd(), p2pSession))
                    {
                        LOG->write(utilities::LogType::Log_Error, FILE_INFO,
                                   "add new online client to SlaveReactor failed, session id: ", p2pSession->sessionId());
                        return;
                    }
                    p2pSession->setSlaveReactorIndex(slaveReactorIndex);
                    LOG->write(utilities::LogType::Log_Info, FILE_INFO, "add new online client to SlaveReactor ",
                               slaveReactorIndex, " successfully, session id: ", p2pSession->sessionId());

                    // 将fd添加到心跳检查器中
                    if (-1 == m_serviceConfig->sessionAliveChecker()->addSession(
                            p2pSession->sessionId(), p2pSession))
                    {
                        LOG->write(utilities::LogType::Log_Error, FILE_INFO,
                                   "add new online client to ClientAliveChecker failed, session id: ",
                                   p2pSession->sessionId());
                        return;
                    }
                    LOG->write(utilities::LogType::Log_Info, FILE_INFO,
                               "add new online client to ClientAliveChecker successfully, session id: ",
                               p2pSession->sessionId());
                });
        });

    // 注册新客户端上线回调
    m_serviceConfig->listenner()->registerConnectHandler([this]() { m_serviceConfig->acceptor()->onConnect(); });

    // 注册HeartbeatBeat处理回调
    m_serviceConfig->sessionServiceDataProcessor()->registerPacketHandler(
        PacketType::PT_HeartBeat,
        [this](const SessionId sessionId, P2PSession::WPtr p2pSessionWeakPtr, PacketHeader::Ptr header,
               PayloadBase::Ptr packet) -> int {
            LOG->write(utilities::LogType::Log_Info, FILE_INFO, "receive heartbeat, session id: ", sessionId);

            // 获取P2PSession
            P2PSession::Ptr p2pSession = p2pSessionWeakPtr.lock();
            if (nullptr == p2pSession)
            {
                LOG->write(utilities::LogType::Log_Info, FILE_INFO, "session may be destroyed, session id: ", sessionId);
                return -1;
            }

            p2pSession->refreshHeartbeat();

            SlaveReactor* slaveReactor = m_serviceConfig->slaveReactorPool()->slaveReactor(p2pSession->slaveReactorIndex());
            assert(nullptr != slaveReactor);

            std::vector<char> buffer = PacketEncodeHelper<PacketType::PT_HeartBeatReply, std::nullopt_t>::encode();
            int ret = slaveReactor->sendData(p2pSession, buffer);
            if (0 != ret)
            {
                LOG->write(utilities::LogType::Log_Error, FILE_INFO, "send heartbeat reply ret: ", ret, ", session id: ",
                           sessionId);
            }
            LOG->write(utilities::LogType::Log_Trace, FILE_INFO, "send heartbeat rely successfully, size: ", buffer.size());

            return ret;
        });

    return 0;
}