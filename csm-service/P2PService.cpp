//
// Created by ChuWen on 9/5/23.
//

#include "P2PService.h"

#include "csm-utilities/Socket.h"
#include "csm-utilities/Logger.h"
#include "csm-utilities/UUIDTool.h"
#include "csm-utilities/TimeTools.h"
#include "protocol/PacketHeader.h"
#include "protocol/payload/PayloadClientInfo.h"
#include "protocol/payload/PayloadClientInfoReply.h"
#include "protocol/payload/PayloadHeartBeat.h"
#include "protocol/payload/PayloadHeartBeatReply.h"
#include "protocol/payload/PayloadModuleMessage.h"

using namespace csm::service;

constexpr std::size_t c_conQueueSize{ 500 };

P2PService::P2PService(ServiceConfig::Ptr serviceConfig) : m_serviceConfig(std::move(serviceConfig))
{}

P2PService::~P2PService()
{
    // 关闭套接字
    utilities::Socket::close(m_fd);
}

int P2PService::init()
{
    LOG->write(utilities::LogType::Log_Info, FILE_INFO, "****************************************TcpServer::init****************************************");

    if (-1 == m_serviceConfig->sessionDispatcher()->init())
    {
        LOG->write(utilities::LogType::Log_Error, FILE_INFO, "session dispatcher init failed");
        return -1;
    }
    LOG->write(utilities::LogType::Log_Info, FILE_INFO, "session dispatcher init successfully");

    if(-1 == m_serviceConfig->sessionDestroyer()->init())
    {
        LOG->write(utilities::LogType::Log_Error, FILE_INFO, "session destroyer init failed");
        return -1;
    }
    LOG->write(utilities::LogType::Log_Info, FILE_INFO, "session destroyer init successfully");

    // 当flag为0的时候，是因为socket发生错误而断开，如果是-1/-2/-3则是握手协议ClientInfoReply包的返回码
    m_serviceConfig->sessionDestroyer()->setHandler([this](const int fd, const int flag){
        LOG->write(utilities::LogType::Log_Info, FILE_INFO, "start destory tcp session, fd: ", fd, ", flag: ", flag);
        // 获取P2pPSession中的协议信息，移除负责共享一条连接协议的信息
        P2PSession::Ptr p2pSession = m_serviceConfig->p2pSessionManager()->session(fd);
        if(nullptr == p2pSession)
        {
            LOG->write(utilities::LogType::Log_Warning, FILE_INFO, "fd may already be removed, fd: ", fd);
            return -1;
        }

        LOG->write(utilities::LogType::Log_Info, FILE_INFO, "set tcp session status to be waiting disconnect, fd: ", fd);
        p2pSession->setWaitingDisconnect(true);

        // 不再检查心跳
        LOG->write(utilities::LogType::Log_Info, FILE_INFO, "remove tcp session from client alive checker, fd: ", fd);
        m_serviceConfig->sessionAliveChecker()->removeSession(fd);

        // 将fd从子Reactor中移除
        LOG->write(utilities::LogType::Log_Info, FILE_INFO, "remove fd from slave reactor, fd: ", fd);
        SlaveReactor::Ptr slaveReactor = getSlaveReactorByFd(fd);
        assert(nullptr != slaveReactor);
        if(-1 == slaveReactor->removeClient(fd))
        {
            LOG->write(utilities::LogType::Log_Error, FILE_INFO, "remove fd from slave reactor epoll failed, fd: ", fd);
        }

        if(ServiceStartType::Node == m_serviceConfig->serviceStartType())
        {
            LOG->write(utilities::LogType::Log_Info, FILE_INFO, "update connect info from host connector, fd: ", fd);
            disconnectClient(p2pSession->peerHostEndPointInfo(), p2pSession->clientId(), p2pSession->handshakeUuid(), flag);
        }

        LOG->write(utilities::LogType::Log_Info, FILE_INFO, "remote fd slave reactor relation, fd: ", fd);
        m_serviceConfig->sessionDispatcher()->removeFdSlaveReactorRelation(fd);
        LOG->write(utilities::LogType::Log_Info, FILE_INFO, "remove tcp session from P2PSessionManager, fd: ", fd);
        m_serviceConfig->p2pSessionManager()->removeSession(fd);

        utilities::Socket::close(fd);

        return 0;
    });

    for(const SlaveReactor::Ptr& slaveReactor : m_serviceConfig->slaveReactors())
    {
        if(-1 == slaveReactor->init())
        {
            LOG->write(utilities::LogType::Log_Error, FILE_INFO, "slave reactor init failed");
            return -1;
        }
        LOG->write(utilities::LogType::Log_Info, FILE_INFO, "slave reactor init successfully");

        slaveReactor->setDisconnectHandler([this](const int fd) { return m_serviceConfig->sessionDestroyer()->addSession(fd); });
        slaveReactor->setSessionDataHandler([this](const int fd, const char* data, const std::size_t dataLen) {
            return m_serviceConfig->sessionDataDecoder()->addSessionData(fd, data, dataLen);
        });
    }

    m_serviceConfig->sessionAliveChecker()->setTimeoutHandler([this](const std::vector<int>& fds){
        std::ranges::for_each(fds, [this](const int fd){ m_serviceConfig->sessionDestroyer()->addSession(fd, 0); });
    });
    m_serviceConfig->sessionAliveChecker()->init();

    // 注册会话数据处理回调
    m_serviceConfig->sessionDataDecoder()->setPacketHandler([this](const int fd, PacketHeader::Ptr header, PayloadBase::Ptr payload) -> int
    {
        if (PacketType::PT_ModuleMessage == header->type())
        {
            m_serviceConfig->sessionModuleDataProcessor()->addPacket(fd, header, payload);
        }
        else
        {
            m_serviceConfig->sessionServiceDataProcessor()->addPacket(fd, header, payload);
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
    m_serviceConfig->sessionModuleDataProcessor()->registerPacketHandler(PacketType::PT_ModuleMessage,
        [this](const int fd, PacketHeader::Ptr header, PayloadBase::Ptr payload) -> int {
            std::uint64_t curTimestamp = utilities::TimeTools::getCurrentTimestamp();
            // 获取fd对应的P2PSession
            P2PSession::Ptr p2pSession = m_serviceConfig->p2pSessionManager()->session(fd);
            // 若任务时间戳小于客户端上线时间戳，任务直接返回不处理，因为可能是客户端离线后新的客户端被分配的相同的fd
            if(nullptr == p2pSession || curTimestamp < p2pSession->clientOnlineTimestamp())
            {
                LOG->write(utilities::LogType::Log_Info, FILE_INFO,
                    "online timestamp: ", p2pSession->clientOnlineTimestamp(), ", curTimestamp: ", curTimestamp);
                return -1;
            }

            PayloadModuleMessage::Ptr payloadModuleMessage = std::dynamic_pointer_cast<PayloadModuleMessage>(payload);

            ModulePacketHandler packetHandler;
            if (0 != m_serviceConfig->modulePacketHandler(header->moduleId(), packetHandler))
            {
                LOG->write(utilities::LogType::Log_Info, FILE_INFO, "module handler not found, moduleId: ", header->moduleId());
                return -1;
            }
            return packetHandler(p2pSession->clientId(), payloadModuleMessage->payload());
    });

    if(-1 == m_serviceConfig->sessionModuleDataProcessor()->init())
    {
        LOG->write(utilities::LogType::Log_Error, FILE_INFO, "session module data processor init failed");
        return -1;
    }
    LOG->write(utilities::LogType::Log_Info, FILE_INFO, "session module data processor init successfully");

    if (-1 == initServer())
    {
        LOG->write(utilities::LogType::Log_Error, FILE_INFO, "init server failed");
        return -1;
    }
    LOG->write(utilities::LogType::Log_Info, FILE_INFO, "init server successfully");

    if(ServiceStartType::Node == m_serviceConfig->serviceStartType())
    {
        if (-1 == initClient())
        {
            LOG->write(utilities::LogType::Log_Error, FILE_INFO, "init client failed");
            return -1;
        }
        LOG->write(utilities::LogType::Log_Info, FILE_INFO, "init client successfully");


    }

    LOG->write(utilities::LogType::Log_Info, FILE_INFO, "****************************************TcpServer::init****************************************");

    return 0;
}

int P2PService::start()
{
    LOG->write(utilities::LogType::Log_Info, FILE_INFO, "****************************************TcpServer::start****************************************");

    // 包解析器
    m_serviceConfig->sessionDataDecoder()->start();
    // 网络模块包处理器
    m_serviceConfig->sessionServiceDataProcessor()->start();
    // 其它模组包处理器
    m_serviceConfig->sessionModuleDataProcessor()->start();
    LOG->write(utilities::LogType::Log_Info, FILE_INFO, "start session data processor successfully");
    std::vector<SlaveReactor::Ptr> slaveReactors = m_serviceConfig->slaveReactors();
    for (SlaveReactor::Ptr &slaveReactor: slaveReactors)
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

    if(ServiceStartType::Node == m_serviceConfig->serviceStartType())
    {
        /*
         * 本节点连接其他节点，对已经连上的节点发送心跳
         * 被连接的节点作为服务端，服务端不向客户端发送心跳
         */
        // 启动host心跳发送服务
        m_serviceConfig->hostsHeartbeatService()->start();
        LOG->write(utilities::LogType::Log_Info, FILE_INFO, "start hosts heartbeat service successfully");
        // 启动host连接服务
        m_serviceConfig->hostsConnector()->start();
        LOG->write(utilities::LogType::Log_Info, FILE_INFO, "start hosts connector successfully");
    }

    LOG->write(utilities::LogType::Log_Info, FILE_INFO, "****************************************TcpServer::start****************************************");

    return 0;
}

int P2PService::stop()
{
    LOG->write(utilities::LogType::Log_Info, FILE_INFO, "****************************************TcpServer::stop****************************************");

    m_serviceConfig->sessionAliveChecker()->stop();
    LOG->write(utilities::LogType::Log_Info, FILE_INFO, "stop client heartbeat checker successfully");

    if(ServiceStartType::Node == m_serviceConfig->serviceStartType())
    {
        m_serviceConfig->hostsHeartbeatService()->stop();
        LOG->write(utilities::LogType::Log_Info, FILE_INFO, "stop hosts heartbeat service successfully");
        m_serviceConfig->hostsConnector()->stop();
        LOG->write(utilities::LogType::Log_Info, FILE_INFO, "stop hosts connector successfully");
    }
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
    std::vector<SlaveReactor::Ptr> slaveReactors = m_serviceConfig->slaveReactors();
    for (SlaveReactor::Ptr &slaveReactor: slaveReactors)
    {
        slaveReactor->stop();
    }
    LOG->write(utilities::LogType::Log_Info, FILE_INFO, "stop all slave reactor successfully");

    LOG->write(utilities::LogType::Log_Info, FILE_INFO, "****************************************TcpServer::stop****************************************");

    return 0;
}

void P2PService::registerModulePacketHandler(csm::protocol::ModuleID moduleId, ModulePacketHandler packetHander)
{
    m_serviceConfig->registerModulePacketHandler(moduleId, std::move(packetHander));
}

bool P2PService::waitAtLeastOneNodeConnected(const int timeout)
{
    return m_serviceConfig->hostsInfoManager()->waitAtLeastOneNodeConnected(timeout);
}

int P2PService::boardcastModuleMessage(csm::protocol::ModuleID moduleId, const std::vector<char>& data)
{
    PacketHeader packetHeader;
    packetHeader.setType(PacketType::PT_ModuleMessage);
    packetHeader.setModuleId(moduleId);
    packetHeader.setPayloadLength(data.size());

    // 编码包为待发送数据
    std::vector<char> buffer;
    buffer.resize(packetHeader.headerLength() + data.size());
    packetHeader.encode(buffer.data(), packetHeader.headerLength());
    memcpy(buffer.data() + packetHeader.headerLength(), data.data(), data.size());

    // 发送给所有在线的节点
    std::vector<std::pair<std::string, int>> allOnlineClients = m_serviceConfig->hostsInfoManager()->getAllOnlineClients();
    for (auto &onlineClient: allOnlineClients)
    {
        if(-1 == sendData(onlineClient.second, buffer))
        {
            LOG->write(utilities::LogType::Log_Error, FILE_INFO, "send data to node: ", onlineClient.first, " failed");
        }
    }

    return 0;
}

int P2PService::sendModuleMessageByNodeId(const NodeId &nodeId, csm::protocol::ModuleID moduleId, const std::vector<char>& data)
{
    PacketHeader packetHeader;
    packetHeader.setType(PacketType::PT_ModuleMessage);
    packetHeader.setModuleId(moduleId);
    packetHeader.setPayloadLength(data.size());

    // 编码包为待发送数据
    std::vector<char> buffer;
    buffer.resize(packetHeader.headerLength() + data.size());
    packetHeader.encode(buffer.data(), packetHeader.headerLength());
    memcpy(buffer.data() + packetHeader.headerLength(), data.data(), data.size());

    // 发送给所有在线的节点
    int fd;
    if (0 != m_serviceConfig->hostsInfoManager()->getHostFdById(nodeId, fd))
    {
        LOG->write(utilities::LogType::Log_Error, FILE_INFO, "send message to ", fd, ", failed, no such node");
        return -1;
    }

    int ret = sendData(fd, buffer);
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
    if(-1 == utilities::Socket::setNonBlock(m_fd))
    {
        return -1;
    }
#endif

    if(-1 == utilities::Socket::setReuseAddr(m_fd))
    {
        LOG->write(utilities::LogType::Log_Error, FILE_INFO, "set socket reuse addr failed, errno: ", errno, ", ", strerror(errno));
        utilities::Socket::close(m_fd);
        return -1;
    }
    LOG->write(utilities::LogType::Log_Info, FILE_INFO, "set socket reuse addr successfully");

    // 绑定监听地址端口
    std::string listenIp;
    unsigned short listenPort;
    if(ServiceStartType::Node == m_serviceConfig->serviceStartType())
    {
        listenIp = m_serviceConfig->nodeConfig()->p2pIp();
        listenPort = m_serviceConfig->nodeConfig()->p2pPort();
    }
    else if(ServiceStartType::RpcServer == m_serviceConfig->serviceStartType())
    {
        listenIp = m_serviceConfig->nodeConfig()->tcpRpcIp();
        listenPort = m_serviceConfig->nodeConfig()->tcpRpcPort();
    }
    else
    {
        LOG->write(utilities::LogType::Log_Error, FILE_INFO, "bind socket failed, errno: ", errno, ", ", strerror(errno));
        return -1;
    }
    if (-1 == utilities::Socket::bind(m_fd, listenIp, listenPort))
    {
        LOG->write(utilities::LogType::Log_Error,
            FILE_INFO, "bind socket failed, errno: ", errno, ", ", strerror(errno), ", listenIp: ", listenIp, ", listenPort:", listenPort);
        utilities::Socket::close(m_fd);
        return -1;
    }
    LOG->write(utilities::LogType::Log_Info, FILE_INFO, "bind socket successfully", ", listenIp: ", listenIp, ", listenPort:", listenPort);

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
    m_serviceConfig->acceptor()->setNewClientCallback([this](const int fd, P2PSession::Ptr p2pSession) {
        // 将fd添加到P2PSession派发器中，由P2PSessionManager决定P2PSession由哪个SlaveReactor管理
        m_serviceConfig->sessionDispatcher()->addSession(fd, [this, fd, p2pSession](const std::size_t slaveReactorIndex){
            LOG->write(utilities::LogType::Log_Info, FILE_INFO, "will dispatch P2PSession to SlaveReactor index: ", slaveReactorIndex, ", fd: ", fd);

            // 将fd添加到心跳检查器中
            if(-1 == m_serviceConfig->sessionAliveChecker()->addSession(fd))
            {
                LOG->write(utilities::LogType::Log_Error, FILE_INFO, "add new online client to ClientAliveChecker failed, fd: ", fd);
                return;
            }
            LOG->write(utilities::LogType::Log_Info, FILE_INFO, "add new online client to ClientAliveChecker successfully, fd: ", fd);

            // 将fd和P2PSession添加到P2PSessionManager中
            if(-1 == m_serviceConfig->p2pSessionManager()->addSession(fd, p2pSession))
            {
                LOG->write(utilities::LogType::Log_Info, FILE_INFO, "add new online client to P2PSessionManager failed, fd: ", fd);
                return;
            }
            LOG->write(utilities::LogType::Log_Info, FILE_INFO, "add new online client to P2PSessionManager successfully, fd: ", fd);

            if(-1 == m_serviceConfig->slaveReactors()[slaveReactorIndex]->addClient(fd))
            {
                LOG->write(utilities::LogType::Log_Error, FILE_INFO, "add new online client to SlaveReactor failed, fd: ", fd);
                m_serviceConfig->p2pSessionManager()->removeSession(fd);
                return;
            }
            LOG->write(utilities::LogType::Log_Info, FILE_INFO, "add new online client to SlaveReactor ", slaveReactorIndex, " successfully, fd: ", fd);
        });
    });

    // 注册新客户端上线回调
    m_serviceConfig->listenner()->registerConnectHandler([this]() { m_serviceConfig->acceptor()->onConnect(); });

    // 注册HeartbeatBeat处理回调
    m_serviceConfig->sessionServiceDataProcessor()->registerPacketHandler(PacketType::PT_HeartBeat, [this](const int fd, PacketHeader::Ptr header, PayloadBase::Ptr packet) -> int {
        // 更新心跳
        m_serviceConfig->sessionAliveChecker()->refreshSessionLastRecvTime(fd);

        PacketHeader packetHeader;
        packetHeader.setType(PacketType::PT_HeartBeatReply);
        packetHeader.setPayloadLength(0);
        std::size_t headerLength = packetHeader.headerLength();

        std::vector<char> buffer;
        buffer.resize(headerLength);
        packetHeader.encode(buffer.data(), headerLength);

        int ret = sendData(fd, buffer);
        if (0 != ret)
        {
            LOG->write(utilities::LogType::Log_Error, FILE_INFO, "send heart beat ret: ", ret, ", fd: ", fd);
        }

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

    m_serviceConfig->hostsConnector()->registerConnectHandler([this](const int fd, P2PSession::Ptr p2pSession) {
        LOG->write(utilities::LogType::Log_Info, FILE_INFO,
                   "connect ", p2pSession->peerHostEndPointInfo().host(), " successfully, ready to dispatch P2PSession to SlaveReactor, fd: ", p2pSession->fd());

        std::string uuid = utilities::UUIDTool::generate();
        p2pSession->setHandshakeUuid(uuid);

        // 因为SlaveReactorManager分配P2PSession到各个SlaveReactor是异步的，所以这里要等待分配完成后发送ClientInfo包
        std::promise<std::size_t> addP2PSessionPromise;
        m_serviceConfig->sessionDispatcher()->addSession(fd, [this, p2pSession, &addP2PSessionPromise](const int slaveReactorIndex) {
            addP2PSessionPromise.set_value(slaveReactorIndex);
        });
        std::size_t slaveReactorIndex = addP2PSessionPromise.get_future().get();

        LOG->write(utilities::LogType::Log_Info, FILE_INFO, "will dispatch P2PSession to SlaveReactor index: ", slaveReactorIndex, ", fd: ", fd);

        if(-1 == m_serviceConfig->sessionAliveChecker()->addSession(fd))
        {
            LOG->write(utilities::LogType::Log_Error, FILE_INFO, "add P2PSession to ClientAliveChecker failed", ", fd: ", fd);
            return;
        }
        LOG->write(utilities::LogType::Log_Info, FILE_INFO, "add P2PSession to ClientAliveChecker successfully", ", fd: ", fd);

        if(-1 == m_serviceConfig->p2pSessionManager()->addSession(fd, p2pSession))
        {
            LOG->write(utilities::LogType::Log_Info, FILE_INFO, "add P2PSession to P2PSessionManager failed", ", fd: ", fd);
            return;
        }
        LOG->write(utilities::LogType::Log_Info, FILE_INFO, "add P2PSession to P2PSessionManager successfully", ", fd: ", fd);

        if(-1 == m_serviceConfig->slaveReactors()[slaveReactorIndex]->addClient(fd))
        {
            LOG->write(utilities::LogType::Log_Error, FILE_INFO, "add session to SlaveReactor, fd: ", fd);
            return;
        }
        LOG->write(utilities::LogType::Log_Info, FILE_INFO, "add P2PSession to SlaveReactor index: ", slaveReactorIndex, " successfully, fd: ", fd);

        LOG->write(utilities::LogType::Log_Info, FILE_INFO, "send ClientInfo payload, fd: ", fd);

        // 发送ClientInfo包
        PayloadClientInfo clientInfoPacket;
        clientInfoPacket.setLocalHost(m_serviceConfig->nodeConfig()->p2pIp() + ":" +
                                      csm::utilities::convertToString(m_serviceConfig->nodeConfig()->p2pPort()));
        clientInfoPacket.setPeerHost(p2pSession->peerHostEndPointInfo().host());
        clientInfoPacket.setHandshakeUuid(uuid);
        clientInfoPacket.setNodeId(m_serviceConfig->nodeConfig()->id());
        std::size_t payloadLength = clientInfoPacket.packetLength();

        PacketHeader packetHeader;
        packetHeader.setType(PacketType::PT_ClientInfo);
        packetHeader.setPayloadLength(clientInfoPacket.packetLength());
        std::size_t headerLength = packetHeader.headerLength();

        std::vector<char> buffer;
        buffer.resize(headerLength + payloadLength);
        packetHeader.encode(buffer.data(), headerLength);
        clientInfoPacket.encode(buffer.data() + headerLength, payloadLength);

        int ret = sendData(fd, buffer);
        LOG->write(utilities::LogType::Log_Info, FILE_INFO, "send ClientInfo ret: ", ret, ", fd: ", fd);
    });

    // 客户端发送ClientInfo包表明自己的身份后，将id与fd绑定，一方面供判定重复连接使用，一方面发送数据的时候通过id查找fd
    m_serviceConfig->sessionServiceDataProcessor()->registerPacketHandler(PacketType::PT_ClientInfo, [this](const int fd, PacketHeader::Ptr header, PayloadBase::Ptr payload) {
        // 收到ClientInfo包，是别人连自己的情况
        PayloadClientInfo::Ptr payloadClientInfo = std::dynamic_pointer_cast<PayloadClientInfo>(payload);

        P2PSession::Ptr p2pSession = m_serviceConfig->p2pSessionManager()->session(fd);
        if(nullptr == p2pSession)
        {
            LOG->write(utilities::LogType::Log_Error, FILE_INFO, "find P2PSession failed, fd: ", fd);
            return -1;
        }

        std::string id = payloadClientInfo->nodeId();
        p2pSession->setClientId(id);
        p2pSession->setHandshakeUuid(payloadClientInfo->handshakeUuid());
        p2pSession->setClientOnlineTimestamp(utilities::TimeTools::getCurrentTimestamp());

        HostEndPointInfo localHostEndPointInfo(payloadClientInfo->localHost());
        HostEndPointInfo peerHostEndPointInfo(payloadClientInfo->peerHost());

        LOG->write(utilities::LogType::Log_Info, FILE_INFO,
                   "recv ClientInfo, fd: ", fd, ", id: ", id, ", localhost: ", localHostEndPointInfo.host(), ", peerHost: ", peerHostEndPointInfo.host());

        /*
         * -1: CommonError
         * -2: ConnectSelf
         * -3: DuplicateConnection
         */
        int replyResult{0};
        if (id == m_serviceConfig->nodeConfig()->id())
        {
            LOG->write(utilities::LogType::Log_Info, FILE_INFO, "connect self, set reply result to -2, then connection initiator will disconnect");
            replyResult = -2;
        }
        else
        {
            int ret = m_serviceConfig->hostsInfoManager()->addHostIdInfo(id, fd, payloadClientInfo->handshakeUuid());
            if (-1 == ret)
            {
                LOG->write(utilities::LogType::Log_Error, FILE_INFO,
                    "host already connected, return -3 then peer will disconnect it, this is who connect me: ", localHostEndPointInfo.host());
                replyResult = -3;
            }
            else
            {
                LOG->write(utilities::LogType::Log_Info, FILE_INFO, "add host id info to HostsInfoManager successfully, fd: ", fd, ", id: ", id);
            }
        }

        // 构造回应包
        PayloadClientInfoReply reply;
        reply.setPeerHost(peerHostEndPointInfo.host());
        reply.setHandshakeUuid(payloadClientInfo->handshakeUuid());
        reply.setNodeId(m_serviceConfig->nodeConfig()->id());
        reply.setResult(replyResult);

        PacketHeader packetHeader;
        packetHeader.setType(PacketType::PT_ClientInfoReply);
        packetHeader.setPayloadLength(reply.packetLength());

        int headerLength = packetHeader.headerLength();
        int payloadLength = reply.packetLength();

        std::vector<char> buffer;
        buffer.resize(headerLength + reply.packetLength());

        packetHeader.encode(buffer.data(), headerLength);
        reply.encode(buffer.data() + headerLength, payloadLength);

        if (-1 == sendData(fd, buffer))
        {
            LOG->write(utilities::LogType::Log_Error, FILE_INFO, "send ClientInfoReply payload failed", ", fd: ", fd, ", id: ", id);
            return -1;
        }

        LOG->write(utilities::LogType::Log_Info, FILE_INFO,
                   "send ClientInfoReply payload successfully, result:", replyResult, ", fd: ", fd, ", id: ", id);

        return 0;
    });

    // 收到ClientInfoReply包
    m_serviceConfig->sessionServiceDataProcessor()->registerPacketHandler(PacketType::PT_ClientInfoReply, [this](const int fd, PacketHeader::Ptr header, PayloadBase::Ptr payload) -> int {
        PayloadClientInfoReply::Ptr payloadClientInfoReply = std::dynamic_pointer_cast<PayloadClientInfoReply>(payload);

        P2PSession::Ptr p2pSession = m_serviceConfig->p2pSessionManager()->session(fd);
        if(nullptr == p2pSession)
        {
            LOG->write(utilities::LogType::Log_Error, FILE_INFO, "find P2PSession failed, fd: ", fd);
            return -1;
        }

        p2pSession->setClientId(payloadClientInfoReply->nodeId());
        p2pSession->setClientOnlineTimestamp(utilities::TimeTools::getCurrentTimestamp());

        HostEndPointInfo peerHostEndPointInfo(p2pSession->peerHostEndPointInfo());
        std::string id = payloadClientInfoReply->nodeId();
        int result = payloadClientInfoReply->result();

        LOG->write(utilities::LogType::Log_Info, FILE_INFO, "recv ClientInfoReply, host: ", peerHostEndPointInfo.host(), ", result: ", result, ", fd: ", fd);

        // 设置Host的id信息
        if (-1 == m_serviceConfig->hostsInfoManager()->setHostId(peerHostEndPointInfo, id))
        {
            LOG->write(utilities::LogType::Log_Error, FILE_INFO, "set host id failed, host: ", peerHostEndPointInfo.host());
            return -1;
        }
        LOG->write(utilities::LogType::Log_Info, FILE_INFO, "set host id successfully, host: ", peerHostEndPointInfo.host(), ", id: ", id, ", fd: ", fd);

        // 告知HostsConnector已经连接上，HostsConnector::setHostConnected内部操作将客户端从正在连接队列中移除
        if (-1 == m_serviceConfig->hostsConnector()->setHostConnectedByFd(fd))
        {
            LOG->write(utilities::LogType::Log_Error, FILE_INFO, "set host connected failed, host: ", peerHostEndPointInfo.host(), ", fd: ", fd);
            return -1;
        }
        LOG->write(utilities::LogType::Log_Info, FILE_INFO, "set host connected successfully, host: ", peerHostEndPointInfo.host(), ", fd: ", fd);

        int anotherConnectionFd{ -1 };
        if (0 != result)
        {
            LOG->write(utilities::LogType::Log_Warning, FILE_INFO, "ClientInfoReply result: ", result, ", fd: ", fd);
            if (-3 == result)
            {
                m_serviceConfig->hostsInfoManager()->getHostFdById(id, anotherConnectionFd);
                LOG->write(utilities::LogType::Log_Warning, FILE_INFO,"get another connection fd: ", anotherConnectionFd, ", fd: ", fd);
                assert(-1 != anotherConnectionFd);

                P2PSession::Ptr anotherP2PSession = m_serviceConfig->p2pSessionManager()->session(anotherConnectionFd);
                assert(nullptr != anotherP2PSession);
                anotherP2PSession->setPeerHostEndPointInfo(payloadClientInfoReply->peerHost());

                LOG->write(utilities::LogType::Log_Warning, FILE_INFO,
                           "another connection fd: ", anotherConnectionFd, ", peer host: ", anotherP2PSession->peerHostEndPointInfo().host(), ", fd: ", fd);
            }
            LOG->write(utilities::LogType::Log_Warning, FILE_INFO, "add tcp session to destoryer, fd: ", fd);
            m_serviceConfig->sessionDestroyer()->addSession(fd, payloadClientInfoReply->result());

            return 0;
        }

        // 可能是双方互相进行连接，但是对方已经发来了ClientInfo包
        std::string anotherConnectionUuid;
        if (true == m_serviceConfig->hostsInfoManager()->isHostIdExist(id, anotherConnectionFd, anotherConnectionUuid))
        {
            if (anotherConnectionUuid < payloadClientInfoReply->handshakeUuid())    // 因为对方的uuid比较小，告诉对方断开与自己的连接，仅保留自己连对方的连接
            {
                // 构造回应包
                PayloadClientInfoReply reply;
                reply.setHandshakeUuid(anotherConnectionUuid);
                reply.setNodeId(m_serviceConfig->nodeConfig()->id());
                reply.setResult(-3);

                PacketHeader packetHeader;
                packetHeader.setType(PacketType::PT_ClientInfoReply);
                packetHeader.setPayloadLength(reply.packetLength());

                int headerLength = packetHeader.headerLength();
                int payloadLength = reply.packetLength();

                std::vector<char> buffer;
                buffer.resize(headerLength + reply.packetLength());

                packetHeader.encode(buffer.data(), headerLength);
                reply.encode(buffer.data() + headerLength, payloadLength);

                if (-1 == sendData(anotherConnectionFd, buffer))
                {
                    LOG->write(utilities::LogType::Log_Error, FILE_INFO,
                               "send ClientInfoReply payload failed", ", fd: ", anotherConnectionFd, ", id: ", id);
                    return -1;
                }

                LOG->write(utilities::LogType::Log_Info, FILE_INFO,
                           "send ClientInfoReply payload successfully", ", fd: ", anotherConnectionFd, ", id: ", id);
            }
            else if (anotherConnectionUuid > payloadClientInfoReply->handshakeUuid())
            {
                return 0;
            }
            else
            {
                // 构造回应包
                PayloadClientInfoReply reply;
                reply.setHandshakeUuid(anotherConnectionUuid);
                reply.setNodeId(m_serviceConfig->nodeConfig()->id());
                reply.setResult(-1);

                PacketHeader packetHeader;
                packetHeader.setType(PacketType::PT_ClientInfoReply);
                packetHeader.setPayloadLength(reply.packetLength());

                std::size_t headerLength = packetHeader.headerLength();
                std::size_t payloadLength = reply.packetLength();

                std::vector<char> buffer;
                buffer.resize(headerLength + reply.packetLength());

                packetHeader.encode(buffer.data(), headerLength);
                reply.encode(buffer.data() + headerLength, payloadLength);

                if (-1 == sendData(anotherConnectionFd, buffer))
                {
                    LOG->write(utilities::LogType::Log_Error, FILE_INFO,
                               "send ClientInfoReply payload failed", ", fd: ", anotherConnectionFd, ", id: ", id);
                    return -1;
                }

                LOG->write(utilities::LogType::Log_Info, FILE_INFO,
                           "send ClientInfoReply payload successfully", ", fd: ", anotherConnectionFd, ", id: ", id);
            }
        }
        else
        {
            /*
             * id是对方的id
             * fd是对端的连接套接字
             * uuid是我方连接对方的uuid
             */
            if (-1 == m_serviceConfig->hostsInfoManager()->addHostIdInfo(id, fd, payloadClientInfoReply->handshakeUuid()))
            {
                LOG->write(utilities::LogType::Log_Error, FILE_INFO, "add id fd relation failed");
            }
        }

        return 0;
    });

    // 注册HeartBeatReply包处理
    m_serviceConfig->sessionServiceDataProcessor()->registerPacketHandler(PacketType::PT_HeartBeatReply, [this](const int fd, PacketHeader::Ptr header, PayloadBase::Ptr payload) -> int {
        m_serviceConfig->sessionAliveChecker()->refreshSessionLastRecvTime(fd);

        return 0;
    });

    // 注册心跳发送回调
    m_serviceConfig->hostsHeartbeatService()->registerHeartbeatSender([this](const int fd, const std::vector<char> &data) {
        if (0 != sendData(fd, data))
        {
            LOG->write(utilities::LogType::Log_Error, FILE_INFO, "send heartbeat failed, to fd: ", fd);
            return -1;
        }

        LOG->write(utilities::LogType::Log_Debug, FILE_INFO, "send heartbeat successfully, to fd: ", fd);

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

    m_serviceConfig->distanceVector()->setPacketSender([this](const std::string& nodeId, const std::vector<char> &data) -> int
    {
        int fd;
        if (0 != m_serviceConfig->hostsInfoManager()->getHostFdById(nodeId, fd))
        {
            LOG->write(utilities::LogType::Log_Error, FILE_INFO, "get host fd from id failed, id: ", nodeId);
            return -1;
        }

        return sendData(fd, data);
    });

    return 0;
}

SlaveReactor::Ptr P2PService::getSlaveReactorByFd(const int fd)
{
    int slaveReactorIndex = m_serviceConfig->sessionDispatcher()->getSlaveReactorIndexByFd(fd);
    if(-1 == slaveReactorIndex)
    {
        LOG->write(utilities::LogType::Log_Error, FILE_INFO, "not found fd: ", fd);
        return nullptr;
    }

    return m_serviceConfig->slaveReactors()[slaveReactorIndex];
}

int P2PService::sendData(const int fd, const std::vector<char> &data)
{
    SlaveReactor::Ptr slaveReactor = getSlaveReactorByFd(fd);
    if(nullptr == slaveReactor)
    {
        LOG->write(utilities::LogType::Log_Error, FILE_INFO, "get SlaveReactor failed, fd: ", fd);
        return -1;
    }

    int ret = slaveReactor->sendData(fd, data);
    if (0 != ret)
    {
        LOG->write(utilities::LogType::Log_Error, FILE_INFO, "send message to ", fd, ", failed, ret: ", ret);
        return -1;
    }
    LOG->write(utilities::LogType::Log_Debug, FILE_INFO, "send message to ", fd, ", successfully");

    return 0;
}

int P2PService::disconnectClient(const HostEndPointInfo &hostEndPointInfo, const std::string &id, const std::string &uuid, const int flag)
{
    LOG->write(utilities::LogType::Log_Info, FILE_INFO, "id: ", id, ", flag: ", flag, ", hostEndPointInfo: ", hostEndPointInfo.host());
    if (0 == flag || -1 == flag)
    {
        if (id == m_serviceConfig->nodeConfig()->id())
        {
            return 0;
        }

        if (-1 == m_serviceConfig->hostsInfoManager()->removeHostIdInfo(id, uuid))
        {
            LOG->write(utilities::LogType::Log_Info, FILE_INFO, "remove host id info failed, id: ", id);
        }
        else
        {
            LOG->write(utilities::LogType::Log_Info, FILE_INFO, "remove host id info successfully, id: ", id);
        }

        if (-1 == m_serviceConfig->hostsInfoManager()->setHostNotConnected(hostEndPointInfo))
        {
            LOG->write(utilities::LogType::Log_Info, FILE_INFO, "set host not connect failed: ", hostEndPointInfo.host());
        }
        else
        {
            LOG->write(utilities::LogType::Log_Info, FILE_INFO, "set host not connect successfully: ", hostEndPointInfo.host());
        }
    }
    else if (-2 == flag)
    {}
    else if (-3 == flag)
    {}

    return 0;
}