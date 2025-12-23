//
// Created by ChuWen on 9/5/23.
//

#include "P2PService.h"

#include "csm-utilities/Logger.h"
#include "csm-utilities/Socket.h"
#include "csm-service/protocol/payload/PayloadClientInfo.h"
#include "csm-service/protocol/payload/PayloadClientInfoReply.h"
#include "csm-service/protocol/payload/PayloadModuleMessage.h"
#include "csm-service/protocol/utilities/PacketEncodeHelper.h"

#include <uuid.h>

using namespace csm::service;

constexpr int c_conQueueSize{500};

P2PService::P2PService(std::unique_ptr<P2PServiceConfig> serviceConfig) :
    m_serviceConfig(std::move(serviceConfig))
{
}

int P2PService::init()
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

            LOG->write(utilities::LogType::Log_Info, FILE_INFO, "update connect info from host connector, session id: ",
                       p2pSession->sessionId());
            disconnectClient(p2pSession->peerHostEndPointInfo(), p2pSession->nodeId(), destoryInfo.flag,
                             p2pSession->sessionId());

            LOG->write(utilities::LogType::Log_Info, FILE_INFO, "remove fd slave reactor relation, session id: ",
                       p2pSession->sessionId());
            m_serviceConfig->sessionDispatcher()->removeSessionIdSlaveReactorRelation(p2pSession->sessionId());

            if (-1 == utilities::Socket::close(p2pSession->fd()))
            {
                LOG->write(utilities::LogType::Log_Error, FILE_INFO, "close fd failed, fd: ", p2pSession->fd(), ", session id: ",
                           p2pSession->sessionId(), ", error: ", errno, ", ", strerror(errno));
                return;
            }
            LOG->write(utilities::LogType::Log_Info, FILE_INFO, "close fd successfully, fd: ", p2pSession->fd(), ", session id: ",
                       p2pSession->sessionId());
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
            if (PacketType::PT_ModuleMessage == header->type())
            {
                m_serviceConfig->sessionModuleDataProcessor()->addPacket(sessionId, std::move(p2pSessionWeakPtr),
                                                                         std::move(header), std::move(payload));
            }
            else
            {
                m_serviceConfig->sessionServiceDataProcessor()->addPacket(sessionId, std::move(p2pSessionWeakPtr),
                                                                          std::move(header), std::move(payload));
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

    // 注册其他模块数据接收回调，SlaveReactor回调包类型和包负载二进制数据
    m_serviceConfig->sessionModuleDataProcessor()->registerPacketHandler(
        PacketType::PT_ModuleMessage,
        [this](const SessionId sessionId, P2PSession::WPtr p2pSessionWeakPtr, PacketHeader::Ptr header,
               PayloadBase::Ptr payload) -> int {
            // 获取P2PSession
            P2PSession::Ptr p2pSession = p2pSessionWeakPtr.lock();
            if (nullptr == p2pSession)
            {
                LOG->write(utilities::LogType::Log_Info, FILE_INFO, "session may be destroyed, session id: ", sessionId);
                return -1;
            }

            PayloadModuleMessage::Ptr payloadModuleMessage = std::dynamic_pointer_cast<PayloadModuleMessage>(payload);

            ModulePacketHandler packetHandler;
            if (0 != m_serviceConfig->modulePacketHandler(header->moduleId(), packetHandler))
            {
                LOG->write(utilities::LogType::Log_Info, FILE_INFO, "module handler not found, moduleId: ", header->moduleId());
                return -1;
            }
            return packetHandler(p2pSession->nodeId(), payloadModuleMessage->payload());
        });

    if (-1 == initServer())
    {
        LOG->write(utilities::LogType::Log_Error, FILE_INFO, "init server failed");
        return -1;
    }
    LOG->write(utilities::LogType::Log_Info, FILE_INFO, "init server successfully");

    if (-1 == initClient())
    {
        LOG->write(utilities::LogType::Log_Error, FILE_INFO, "init client failed");
        return -1;
    }
    LOG->write(utilities::LogType::Log_Info, FILE_INFO, "init client successfully");

    if (0 != initDistanceVector())
    {
        LOG->write(utilities::LogType::Log_Error, FILE_INFO, "init distance vector failed");
        return -1;
    }
    LOG->write(utilities::LogType::Log_Info, FILE_INFO, "init distance vector successfully");

    LOG->write(utilities::LogType::Log_Info, FILE_INFO,
               "****************************************TcpServer::init****************************************");

    return 0;
}

int P2PService::start()
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

    /*
     * 本节点连接其他节点，对已经连上的节点发送心跳
     * 被连接的节点作为服务端，服务端不向客户端发送心跳
     */
    // 启动host心跳发送服务
    m_serviceConfig->hostsHeartbeatService()->start();
    LOG->write(utilities::LogType::Log_Info, FILE_INFO, "start hosts heartbeat service successfully");

    // 启动距离向量服务
    m_serviceConfig->distanceVector()->start();
    LOG->write(utilities::LogType::Log_Info, FILE_INFO, "start distance vector successfully");

    // 启动host连接服务
    m_serviceConfig->hostsConnector()->start();
    LOG->write(utilities::LogType::Log_Info, FILE_INFO, "start hosts connector successfully");

    LOG->write(utilities::LogType::Log_Info, FILE_INFO,
               "****************************************TcpServer::start****************************************");

    return 0;
}

int P2PService::stop()
{
    LOG->write(utilities::LogType::Log_Info, FILE_INFO,
               "****************************************TcpServer::stop****************************************");

    m_serviceConfig->sessionAliveChecker()->stop();
    LOG->write(utilities::LogType::Log_Info, FILE_INFO, "stop client heartbeat checker successfully");
    m_serviceConfig->hostsHeartbeatService()->stop();
    LOG->write(utilities::LogType::Log_Info, FILE_INFO, "stop hosts heartbeat service successfully");
    m_serviceConfig->hostsConnector()->stop();
    LOG->write(utilities::LogType::Log_Info, FILE_INFO, "stop hosts connector successfully");

    m_serviceConfig->distanceVector()->stop();
    LOG->write(utilities::LogType::Log_Info, FILE_INFO, "stop distance vector successfully");

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

void P2PService::registerModulePacketHandler(csm::protocol::ModuleID moduleId, ModulePacketHandler packetHander)
{
    m_serviceConfig->registerModulePacketHandler(moduleId, std::move(packetHander));
}

int P2PService::boardcastModuleMessage(csm::protocol::ModuleID moduleId, const std::vector<char>& data)
{
    std::vector<char> buffer = PacketEncodeHelper<PacketType::PT_ModuleMessage, std::vector<char>>::encode(moduleId, data);

    // 发送给所有在线的节点
    std::vector<std::pair<std::string, SessionId>> allOnlineHosts = m_serviceConfig->hostsInfoManager()->getAllHosts();
    for (const auto& host : allOnlineHosts)
    {
        if (-1 == sendData(host.second, buffer))
        {
            LOG->write(utilities::LogType::Log_Error, FILE_INFO, "send data to node: ", host.first, " failed");
        }
    }

    return 0;
}

int P2PService::sendModuleMessageByNodeId(const NodeId& nodeId, csm::protocol::ModuleID moduleId,
                                          const std::vector<char>& data)
{
    std::vector<char> buffer = PacketEncodeHelper<PacketType::PT_ModuleMessage, std::vector<char>>::encode(moduleId, data);

    SessionId sessionId;
    if (0 != m_serviceConfig->hostsInfoManager()->getSessionId(nodeId, sessionId))
    {
        LOG->write(utilities::LogType::Log_Error, FILE_INFO, "send message to ", sessionId, ", failed, no such node");
        return -1;
    }

    int ret = sendData(sessionId, buffer);
    if (0 != ret)
    {
        LOG->write(utilities::LogType::Log_Error, FILE_INFO, "send message to ", nodeId, ", failed, ret: ", ret);
        return -1;
    }
    LOG->write(utilities::LogType::Log_Debug, FILE_INFO, "send message to ", nodeId, ", successfully");

    return 0;
}

int P2PService::initServer()
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
                p2pSession,
                [this, p2pSession](const std::size_t slaveReactorIndex) {
                    LOG->write(utilities::LogType::Log_Info, FILE_INFO,
                               "will dispatch P2PSession to SlaveReactor index: ", slaveReactorIndex, ", session id: ",
                               p2pSession->sessionId());

                    SlaveReactor* slaveReactor = m_serviceConfig->slaveReactorPool()->slaveReactor(slaveReactorIndex);
                    if (-1 == slaveReactor->addSession(
                            p2pSession->fd(), p2pSession))
                    {
                        LOG->write(utilities::LogType::Log_Error, FILE_INFO,
                                   "add new online client to SlaveReactor failed, session id: ", p2pSession->sessionId());
                        return;
                    }
                    p2pSession->setSlaveReactorIndex(slaveReactorIndex);
                    LOG->write(utilities::LogType::Log_Info, FILE_INFO, "add new online client to SlaveReactor ",
                               slaveReactorIndex, " successfully, session id: ", p2pSession->sessionId());

                    // 将fd添加到心跳检查器中
                    if (-1 == m_serviceConfig->sessionAliveChecker()->addSession(p2pSession->sessionId(), p2pSession))
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
            LOG->write(utilities::LogType::Log_Debug, FILE_INFO, "receive heartbeat, session id: ", sessionId);

            // 获取P2PSession
            P2PSession::Ptr p2pSession = p2pSessionWeakPtr.lock();
            if (nullptr == p2pSession)
            {
                LOG->write(utilities::LogType::Log_Info, FILE_INFO, "session may be destroyed, session id: ", sessionId);
                return -1;
            }

            p2pSession->refreshHeartbeat();

            std::vector<char> buffer = PacketEncodeHelper<PacketType::PT_HeartBeatReply, std::nullopt_t>::encode();
            int ret = sendData(sessionId, buffer);
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

int P2PService::initClient()
{
    if (-1 == m_serviceConfig->hostsInfoManager()->init())
    {
        LOG->write(utilities::LogType::Log_Error, FILE_INFO, "HostsInfoManager init failed");
        return -1;
    }
    LOG->write(utilities::LogType::Log_Info, FILE_INFO, "HostInfoManager init successfully");

    if (-1 == m_serviceConfig->hostsConnector()->init())
    {
        LOG->write(utilities::LogType::Log_Error, FILE_INFO, "HostsConnector init failed");
        return -1;
    }
    LOG->write(utilities::LogType::Log_Info, FILE_INFO, "HostsConnector init successfully");

    if (-1 == m_serviceConfig->hostsHeartbeatService()->init())
    {
        LOG->write(utilities::LogType::Log_Error, FILE_INFO, "HostsHeartbeatService init failed");
        return -1;
    }
    LOG->write(utilities::LogType::Log_Info, FILE_INFO, "HostsHeartbeatService init successfully");

    m_serviceConfig->hostsConnector()->registerConnectHandler(
        [this](P2PSession::Ptr p2pSession) {
            LOG->write(utilities::LogType::Log_Info, FILE_INFO, "connect ", p2pSession->peerHostEndPointInfo().host(),
                       " successfully, ready to dispatch P2PSession to SlaveReactor", ", session id: ",
                       p2pSession->sessionId());

            // 因为SlaveReactorManager分配P2PSession到各个SlaveReactor是异步的，所以这里要等待分配完成后发送ClientInfo包
            std::promise<std::size_t> addP2PSessionPromise;
            m_serviceConfig->sessionDispatcher()->addSession(
                p2pSession, [&addP2PSessionPromise](const int slaveReactorIndex) {
                    addP2PSessionPromise.set_value(slaveReactorIndex);
                });
            std::size_t slaveReactorIndex = addP2PSessionPromise.get_future().get();

            LOG->write(utilities::LogType::Log_Info, FILE_INFO,
                       "will dispatch P2PSession to SlaveReactor index: ", slaveReactorIndex, ", session id: ",
                       p2pSession->sessionId());

            if (-1 == m_serviceConfig->sessionAliveChecker()->addSession(p2pSession->sessionId(), p2pSession))
            {
                LOG->write(utilities::LogType::Log_Error, FILE_INFO, "add P2PSession to ClientAliveChecker failed",
                           ", session id: ", p2pSession->sessionId());
                return;
            }
            LOG->write(utilities::LogType::Log_Info, FILE_INFO, "add P2PSession to ClientAliveChecker successfully",
                       ", session id: ", p2pSession->sessionId());

            SlaveReactor* slaveReactor = m_serviceConfig->slaveReactorPool()->slaveReactor(slaveReactorIndex);
            if (-1 == slaveReactor->addSession(p2pSession->fd(), p2pSession))
            {
                LOG->write(utilities::LogType::Log_Error, FILE_INFO, "add session to SlaveReactor failed, session id: ",
                           p2pSession->sessionId());
                return;
            }
            p2pSession->setSlaveReactorIndex(slaveReactorIndex);
            LOG->write(utilities::LogType::Log_Info, FILE_INFO, "add P2PSession to SlaveReactor index: ", slaveReactorIndex,
                       " successfully, session id: ", p2pSession->sessionId());

            LOG->write(utilities::LogType::Log_Info, FILE_INFO, "send ClientInfo payload, session id: ", p2pSession->sessionId());

            // 发送ClientInfo包
            PayloadClientInfo clientInfoPacket;
            clientInfoPacket.setLocalHost(m_serviceConfig->nodeConfig()->p2pIp() + ":" +
                                          utilities::convertToString(m_serviceConfig->nodeConfig()->p2pPort()));
            clientInfoPacket.setPeerHost(p2pSession->peerHostEndPointInfo().host());
            clientInfoPacket.setNodeId(m_serviceConfig->nodeConfig()->nodeId());

            std::vector<char> buffer = PacketEncodeHelper<PacketType::PT_ClientInfo, PayloadClientInfo>::encode(clientInfoPacket);
            int ret = sendData(p2pSession->sessionId(), buffer);
            if (0 != ret)
            {
                LOG->write(utilities::LogType::Log_Error, FILE_INFO, "send ClientInfo ret: ", ret, ", session id: ",
                           p2pSession->sessionId());
                return;
            }
            LOG->write(utilities::LogType::Log_Info, FILE_INFO, "send ClientInfo ret: ", ret, ", session id: ",
                       p2pSession->sessionId());
        });

    // 客户端发送ClientInfo包表明自己的身份后，将id与fd绑定，一方面供判定重复连接使用，一方面发送数据的时候通过id查找fd
    m_serviceConfig->sessionServiceDataProcessor()->registerPacketHandler(
        PacketType::PT_ClientInfo,
        [this](const SessionId sessionId, P2PSession::WPtr p2pSessionWeakPtr, PacketHeader::Ptr header,
               PayloadBase::Ptr payload) {
            // 获取P2PSession
            P2PSession::Ptr p2pSession = p2pSessionWeakPtr.lock();
            if (nullptr == p2pSession)
            {
                LOG->write(utilities::LogType::Log_Info, FILE_INFO, "session may be destroyed, session id: ", sessionId);
                return -1;
            }

            // 收到ClientInfo包，是别人连自己的情况
            PayloadClientInfo::Ptr payloadClientInfo = std::dynamic_pointer_cast<PayloadClientInfo>(payload);

            std::string nodeId = payloadClientInfo->nodeId();
            p2pSession->setNodeId(nodeId);

            HostEndPointInfo localHostEndPointInfo(payloadClientInfo->localHost());
            HostEndPointInfo peerHostEndPointInfo(payloadClientInfo->peerHost());

            LOG->write(utilities::LogType::Log_Info, FILE_INFO, "recv ClientInfo, session id: ", sessionId, ", node id: ", nodeId,
                       ", localhost: ", localHostEndPointInfo.host(), ", peerHost: ", peerHostEndPointInfo.host());

            /*
             * -1: CommonError
             * -2: ConnectSelf
             * -3: DuplicateConnection_Serial
             * -4: DuplicateConnection_Parallel
             */
            int replyResult{0};
            if (nodeId == m_serviceConfig->nodeConfig()->nodeId())
            {
                LOG->write(utilities::LogType::Log_Info, FILE_INFO,
                           "connect self, set reply result to -2, then connection initiator will disconnect");
                replyResult = -2;
            }
            else
            {
                int ret = m_serviceConfig->hostsInfoManager()->addHostIdInfo(nodeId, sessionId);
                if (-1 == ret)
                {
                    LOG->write(utilities::LogType::Log_Error, FILE_INFO,
                               "host already connected, return -3 then peer will disconnect it, this is who connect me: ",
                               localHostEndPointInfo.host());
                    replyResult = -3;
                }
                else
                {
                    LOG->write(utilities::LogType::Log_Info, FILE_INFO,
                               "add host id info to HostsInfoManager successfully, session id: ", sessionId, ", node id: ",
                               nodeId);
                }
            }

            // 构造回应包
            PayloadClientInfoReply clientInfoReply;
            clientInfoReply.setPeerHost(peerHostEndPointInfo.host());
            clientInfoReply.setHandshakeUuid(payloadClientInfo->handshakeUuid());
            clientInfoReply.setNodeId(m_serviceConfig->nodeConfig()->nodeId());
            clientInfoReply.setResult(replyResult);

            std::vector<char> buffer =
                PacketEncodeHelper<PacketType::PT_ClientInfoReply, PayloadClientInfoReply>::encode(clientInfoReply);
            if (-1 == sendData(sessionId, buffer))
            {
                LOG->write(utilities::LogType::Log_Error, FILE_INFO, "send ClientInfoReply payload failed", ", session id: ",
                           sessionId,
                           ", node id: ", nodeId);
                return -1;
            }

            LOG->write(utilities::LogType::Log_Info, FILE_INFO, "send ClientInfoReply payload successfully, result:", replyResult,
                       ", session id: ", sessionId, ", node id: ", nodeId);

            return 0;
        });

    // 收到ClientInfoReply包
    m_serviceConfig->sessionServiceDataProcessor()->registerPacketHandler(
        PacketType::PT_ClientInfoReply,
        [this](const SessionId sessionId, P2PSession::WPtr p2pSessionWeakPtr, PacketHeader::Ptr header,
               PayloadBase::Ptr payload) -> int {
            P2PSession::Ptr p2pSession = p2pSessionWeakPtr.lock();
            if (nullptr == p2pSession)
            {
                LOG->write(utilities::LogType::Log_Error, FILE_INFO, "session may be destoryed, session id: ", sessionId);
                return -1;
            }

            PayloadClientInfoReply::Ptr payloadClientInfoReply = std::dynamic_pointer_cast<PayloadClientInfoReply>(payload);

            p2pSession->setNodeId(payloadClientInfoReply->nodeId());

            HostEndPointInfo peerHostEndPointInfo(p2pSession->peerHostEndPointInfo());
            std::string id = payloadClientInfoReply->nodeId();
            int result = payloadClientInfoReply->result();

            LOG->write(utilities::LogType::Log_Info, FILE_INFO, "recv ClientInfoReply, host: ", peerHostEndPointInfo.host(),
                       ", result: ", result, ", session id: ", sessionId);

            if (0 != result)
            {
                LOG->write(utilities::LogType::Log_Warning, FILE_INFO, "ClientInfoReply result: ", result, ", session id: ",
                           sessionId);

                if (-4 == result)
                {
                    SessionId anotherSessionId{0};
                    int ret = m_serviceConfig->hostsInfoManager()->getSessionId(id, anotherSessionId);
                    assert(0 == ret);

                    P2PSession::Ptr anotherP2PSession = getP2PSession(anotherSessionId);
                    assert(nullptr != anotherP2PSession);

                    LOG->write(utilities::LogType::Log_Info, FILE_INFO, "get another connection session id: ", anotherSessionId,
                               ", peer host: ", anotherP2PSession->peerHostEndPointInfo().host(),
                               ", session id: ", sessionId);
                    anotherP2PSession->setPeerHostEndPointInfo(payloadClientInfoReply->peerHost());
                }

                LOG->write(utilities::LogType::Log_Info, FILE_INFO, "add tcp session to destoryer, session id: ", sessionId);
                m_serviceConfig->sessionDestroyer()->addSessions(
                    {SessionDestoryInfo(sessionId, p2pSessionWeakPtr, payloadClientInfoReply->result(), false)});

                return 0;
            }

            LOG->write(utilities::LogType::Log_Info, FILE_INFO, "recv ClientInfoReply, host: ", peerHostEndPointInfo.host(),
                       ", result: ", result, ", session id: ", sessionId);

            // 可能是双方互相进行连接，但是对方已经发来了ClientInfo包
            SessionId anotherSessionId{0};
            if (0 == m_serviceConfig->hostsInfoManager()->getSessionId(id, anotherSessionId))
            {
                if (m_serviceConfig->nodeConfig()->nodeId() < payloadClientInfoReply->nodeId())
                {
#if 0
                    // 因为对方的uuid比较小，告诉对方断开与自己的连接，仅保留自己连对方的连接
                    // 构造回应包
                    PayloadClientInfoReply clientInfoReply;
                    clientInfoReply.setNodeId(m_serviceConfig->nodeConfig()->nodeId());
                    clientInfoReply.setResult(-4);

                    std::vector<char> buffer =
                        PacketEncodeHelper<PacketType::PT_ClientInfoReply, PayloadClientInfoReply>::encode(clientInfoReply);
                    if (-1 == sendData(anotherSessionId, buffer))
                    {
                        LOG->write(utilities::LogType::Log_Error, FILE_INFO, "send ClientInfoReply payload failed",
                                   ", session id: ", sessionId, ", node id: ", id);
                        return -1;
                    }

                    LOG->write(utilities::LogType::Log_Info, FILE_INFO, "send ClientInfoReply payload successfully",
                               ", session id: ", sessionId, ", node id: ", id);
#endif

                    LOG->write(utilities::LogType::Log_Info, FILE_INFO, "add tcp session to destoryer, session id: ", sessionId);
                    m_serviceConfig->sessionDestroyer()->addSessions(
                        {SessionDestoryInfo(sessionId, p2pSessionWeakPtr, -4, false)});
                }
                else
                {
                    return 0;
                }
            }
            else
            {
                LOG->write(utilities::LogType::Log_Info, FILE_INFO, "set host id start, host: ", peerHostEndPointInfo.host(),
                           ", node id: ", id, ", session id: ", sessionId);

                // 设置Host的id信息
                if (-1 == m_serviceConfig->hostsInfoManager()->setHostId(peerHostEndPointInfo, id))
                {
                    LOG->write(utilities::LogType::Log_Error, FILE_INFO,
                               "set host id failed, host: ", peerHostEndPointInfo.host());
                    return -1;
                }
                LOG->write(utilities::LogType::Log_Info, FILE_INFO,
                           "set host id successfully, host: ", peerHostEndPointInfo.host(), ", id: ", id, ", session id: ",
                           sessionId);

                // 告知HostsConnector已经连接上，HostsConnector::setHostConnected内部操作将客户端从正在连接队列中移除
                if (-1 == m_serviceConfig->hostsConnector()->setHostConnected(peerHostEndPointInfo))
                {
                    LOG->write(utilities::LogType::Log_Error, FILE_INFO,
                               "set host connected failed, host: ", peerHostEndPointInfo.host(), ", session id: ", sessionId);
                    return -1;
                }
                LOG->write(utilities::LogType::Log_Info, FILE_INFO,
                           "set host connected successfully, host: ", peerHostEndPointInfo.host(), ", session id: ", sessionId);

                /*
                 * id是对方的id
                 * fd是对端的连接套接字
                 * uuid是我方连接对方的uuid
                 */
                if (-1 == m_serviceConfig->hostsInfoManager()->addHostIdInfo(id, sessionId))
                {
                    LOG->write(utilities::LogType::Log_Error, FILE_INFO, "add node id and session id relation failed");
                    return -1;
                }
            }

            return 0;
        });

    // 注册HeartBeatReply包处理
    m_serviceConfig->sessionServiceDataProcessor()->registerPacketHandler(
        PacketType::PT_HeartBeatReply,
        [this](const SessionId sessionId, P2PSession::WPtr p2pSessionWeakPtr, PacketHeader::Ptr header,
               PayloadBase::Ptr payload) -> int {
            LOG->write(utilities::LogType::Log_Debug, FILE_INFO, "receive heartbeat reply, session id: ", sessionId);

            P2PSession::Ptr p2pSession = p2pSessionWeakPtr.lock();
            if (nullptr == p2pSession)
            {
                LOG->write(utilities::LogType::Log_Error, FILE_INFO, "session may be destoryed, session id: ", sessionId);
                return -1;
            }

            p2pSession->refreshHeartbeat();

            return 0;
        });

    // 注册心跳发送回调
    m_serviceConfig->hostsHeartbeatService()->registerHeartbeatSender(
        [this](const SessionId& sessionId, const std::vector<char>& data) {
            if (0 != sendData(sessionId, data))
            {
                LOG->write(utilities::LogType::Log_Error, FILE_INFO, "send heartbeat failed, to session id: ", sessionId);
                return -1;
            }

            LOG->write(utilities::LogType::Log_Debug, FILE_INFO, "send heartbeat successfully, to session id: ", sessionId);

            return 0;
        });

    return 0;
}

int P2PService::initDistanceVector()
{
    if (0 != m_serviceConfig->distanceVector()->init())
    {
        LOG->write(utilities::LogType::Log_Error, FILE_INFO, "init distance vector failed");
        return -1;
    }
    LOG->write(utilities::LogType::Log_Info, FILE_INFO, "init distance vector successfully");

    m_serviceConfig->distanceVector()->setPacketSender(
        [this](const std::string& nodeId, const std::vector<char>& data) -> int {
            SessionId sessionId;
            if (0 != m_serviceConfig->hostsInfoManager()->getSessionId(nodeId, sessionId))
            {
                LOG->write(utilities::LogType::Log_Error, FILE_INFO, "get host fd from id failed, id: ", nodeId);
                return -1;
            }

            return sendData(sessionId, data);
        });

    return 0;
}

P2PSession::Ptr P2PService::getP2PSession(SessionId sessionId)
{
    int slaveReactorIndex = m_serviceConfig->sessionDispatcher()->getSlaveReactorIndexBySessionId(sessionId);
    if (-1 == slaveReactorIndex)
    {
        return nullptr;
    }

    SlaveReactor* slaveReactor = m_serviceConfig->slaveReactorPool()->slaveReactor(slaveReactorIndex);
    return slaveReactor->getP2PSession(sessionId);
}

int P2PService::sendData(const SessionId sessionId, const std::vector<char>& data)
{
    int slaveReactorIndex = m_serviceConfig->sessionDispatcher()->getSlaveReactorIndexBySessionId(sessionId);
    if (-1 == slaveReactorIndex)
    {
        LOG->write(utilities::LogType::Log_Error, FILE_INFO, "session not found, session id: ", sessionId);
        return -1;
    }

    SlaveReactor* slaveReactor = m_serviceConfig->slaveReactorPool()->slaveReactor(slaveReactorIndex);
    int ret = slaveReactor->sendData(sessionId, data);
    if (0 != ret)
    {
        LOG->write(utilities::LogType::Log_Error, FILE_INFO, "send message to ", sessionId, ", failed, ret: ", ret);
        return -1;
    }
    LOG->write(utilities::LogType::Log_Debug, FILE_INFO, "send message to ", sessionId, ", successfully");

    return 0;
}

void P2PService::disconnectClient(const HostEndPointInfo& hostEndPointInfo, const std::string& nodeId,
                                  const int flag, const SessionId& sessionId)
{
    // 当flag为0的时候，是因为socket发生错误而断开，如果是-1/-2/-3/-4则是握手协议ClientInfoReply包的返回码
    LOG->write(utilities::LogType::Log_Info, FILE_INFO, "node id: ", nodeId, ", flag: ", flag,
               ", hostEndPointInfo: ", hostEndPointInfo.host(), ", session id: ", sessionId);

    if (0 == flag || -1 == flag)
    {
        if (nodeId == m_serviceConfig->nodeConfig()->nodeId())
        {
            return;
        }

        if (-1 == m_serviceConfig->hostsInfoManager()->removeHostIdInfo(nodeId, sessionId))
        {
            LOG->write(utilities::LogType::Log_Info, FILE_INFO, "remove host id info failed, node id: ", nodeId, ", fd: ",
                       sessionId);
        }
        else
        {
            LOG->write(utilities::LogType::Log_Info, FILE_INFO, "remove host id info successfully, node id: ", nodeId, ", fd: ",
                       sessionId);
        }

        if (-1 == m_serviceConfig->hostsInfoManager()->setHostNotConnected(hostEndPointInfo))
        {
            LOG->write(utilities::LogType::Log_Info, FILE_INFO, "set host not connect failed: ", hostEndPointInfo.host(),
                       ", session id: ", sessionId);
        }
        else
        {
            LOG->write(utilities::LogType::Log_Info, FILE_INFO, "set host not connect successfully: ", hostEndPointInfo.host(),
                       ", session id: ", sessionId);
        }
    }
    else if (-2 == flag)
    {
    }
    else if (-3 == flag)
    {
    }
    else if (-4 == flag)
    {
        if (-1 == m_serviceConfig->hostsInfoManager()->removeHostIdInfo(nodeId, sessionId))
        {
            LOG->write(utilities::LogType::Log_Error, FILE_INFO, "remove host id info failed, node id: ", nodeId, ", session id: ",
                       sessionId);
        }
        else
        {
            LOG->write(utilities::LogType::Log_Info, FILE_INFO, "remove host id info successfully, node id: ", nodeId,
                       ", session id: ", sessionId);
        }
    }
}