//
// Created by ChuWen on 9/5/23.
//

#include "TcpService.h"

#include <utility>
#include <ranges>
#include "csm-utilities/Socket.h"
#include "csm-utilities/Logger.h"
#include "csm-utilities/UUIDTool.h"
#include "protocol/PacketHeader.h"
#include "protocol/packet/PacketClientInfo.h"
#include "protocol/packet/PacketClientInfoReply.h"

using namespace csm::service;

constexpr std::size_t c_conQueueSize{500};

TcpService::TcpService(ServiceConfig::Ptr serviceConfig) : m_serviceConfig(std::move(serviceConfig))
{}

int TcpService::init()
{
    if (-1 == m_serviceConfig->sessionDispatcher()->init())
    {
        LOG->write(utilities::LogType::Log_Error, FILE_INFO, "session dispatcher init failed");
        utilities::Socket::close(m_fd);
        return -1;
    }
    LOG->write(utilities::LogType::Log_Info, FILE_INFO, "session dispatcher init successfully");

    if(-1 == m_serviceConfig->sessionDestroyer()->init())
    {
        LOG->write(utilities::LogType::Log_Error, FILE_INFO, "session destroyer init failed");
        utilities::Socket::close(m_fd);
        return -1;
    }
    LOG->write(utilities::LogType::Log_Info, FILE_INFO, "session destroyer init successfully");

    // 初始化包处理线程池
    if(-1 == m_serviceConfig->packetProcessor()->init(m_serviceConfig->nodeConfig()->packetProcessThreadNum(), "packet_proc"))
    {
        LOG->write(utilities::LogType::Log_Error, FILE_INFO, "packet processor init failed");
        utilities::Socket::close(m_fd);
        return -1;
    }
    LOG->write(utilities::LogType::Log_Info, FILE_INFO, "packet processor init successfully");

    // 当flag为0的时候，是因为socket发生错误而断开，如果是-1/-2/-3则是握手协议ClientInfoReply包的返回码
    m_serviceConfig->sessionDestroyer()->setHandler([this](const int fd, const int flag){
        // 不再检查心跳
        m_serviceConfig->clientAliveChecker()->removeClient(fd);

        // 将fd从子Reactor中移除
        SlaveReactor::Ptr slaveReactor = getSlaveReactorByFd(fd);
        assert(nullptr != slaveReactor);
        slaveReactor->removeClient(fd);

        // 获取TcpSession中的协议信息，移除负责共享一条连接协议的信息
        TcpSession::Ptr tcpSession = m_serviceConfig->tcpSessionManager()->tcpSession(fd);
        assert(nullptr != tcpSession);
        disconnectClient(tcpSession->peerHostEndPointInfo(), tcpSession->getClientId(), tcpSession->handshakeUuid(), flag);

        m_serviceConfig->sessionDispatcher()->removeFdSlaveReactorRelation(fd);
        m_serviceConfig->tcpSessionManager()->removeTcpSession(fd);

        return 0;
    });

    // 注册数据接收回调，SlaveReactor回调包类型和包负载二进制数据
    m_serviceConfig->slaveReactorManager()->registerModuleMessageHandler(
            [this](const int fd, const std::int32_t moduleId, std::shared_ptr<std::vector<char>> &payloadData) {
                std::uint64_t curTimestamp = utilities::Timestamp::getCurrentTimestamp();
                const auto expression = [fd, curTimestamp, moduleId, payloadData, this]() {
                    // 获取fd对应的TcpSession
                    TcpSession::Ptr tcpSession = m_serviceConfig->tcpSessionManager()->tcpSession(fd);
                    // 若任务时间戳小于客户端上线时间戳，任务直接返回不处理，因为可能是客户端离线后新的客户端被分配的相同的fd
                    if(nullptr == tcpSession || curTimestamp < tcpSession->getClientOnlineTimestamp())
                    {
                        LOG->write(utilities::LogType::Log_Info, FILE_INFO,
                                   "online timestamp: ", tcpSession->getClientOnlineTimestamp(), ", curTimestamp: ", curTimestamp);
                        return -1;
                    }

                    auto iter = m_modulePacketHandler.find(moduleId);
                    if (m_modulePacketHandler.end() == iter)
                    {
                        LOG->write(utilities::LogType::Log_Info, FILE_INFO, "module handler not found, moduleId: ", moduleId);
                        return -1;
                    }
                    return iter->second(payloadData);

#if 0
                    if(nullptr != writeHandler)
                    {
                        // 将回应包写入发送缓冲区
                        LOG->write(utilities::LogType::Log_Trace, FILE_INFO,
                                                                                     "try write reply packet to buffer, write size: ", sumLength, ", fd: ", fd);
                        int ret{0};
                        while(0 != (ret = writeHandler(fd, buffer)))
                        {
                            if(-2 == ret)
                            {
                                // 客户端已经离线
                                LOG->write(utilities::LogType::Log_Trace, FILE_INFO,
                                                                                             " client offline, fd: ", fd);
                                break;
                            }
                            else if(-1 == ret)
                            {
                                // 发送缓冲区满，等待10ms再次发送
                                std::this_thread::sleep_for(std::chrono::milliseconds(10));
                            }
                        }
                        LOG->write(utilities::LogType::Log_Trace, FILE_INFO,
                                                                                     " write reply packet to buffer successfully, write size: ", sumLength, ", fd: ", fd);
                    }
#endif

                    return 0;
                };
                m_serviceConfig->packetProcessor()->push(expression);
            });

    // 注册客户端离线回调
    m_serviceConfig->slaveReactorManager()->registerDisconnectHandler(
            std::bind(&TcpService::onClientDisconnect, this, std::placeholders::_1,
                      std::placeholders::_2, std::placeholders::_3, std::placeholders::_4));

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

    return 0;
}

int TcpService::uninit()
{
    if (-1 == m_serviceConfig->sessionDispatcher()->uninit())
    {
        LOG->write(utilities::LogType::Log_Error, FILE_INFO, "uninit SessionDispatcher failed");
        return -1;
    }
    LOG->write(utilities::LogType::Log_Info, FILE_INFO, "uninit SessionDispatcher successfully");

    if (-1 == m_serviceConfig->packetProcessor()->uninit())
    {
        LOG->write(utilities::LogType::Log_Error, FILE_INFO, "uninit PacketProcessThreadPool failed");
        return -1;
    }
    LOG->write(utilities::LogType::Log_Info, FILE_INFO, "uninit PacketProcessThreadPool successfully");

    if (-1 == uninitServer())
    {
        LOG->write(utilities::LogType::Log_Error, FILE_INFO, "uninit server failed");
        return -1;
    }
    LOG->write(utilities::LogType::Log_Info, FILE_INFO, "uninit server successfully");

    if (-1 == uninitClient())
    {
        LOG->write(utilities::LogType::Log_Error, FILE_INFO, "uninit client failed");
        return -1;
    }
    LOG->write(utilities::LogType::Log_Info, FILE_INFO, "uninit client successfully");

    return 0;
}

int TcpService::start()
{
    // 包处理线程池
    m_serviceConfig->packetProcessor()->start();
    LOG->write(utilities::LogType::Log_Info, FILE_INFO, "start packet processor successfully");
    std::vector<SlaveReactor::Ptr> slaveReactors = m_serviceConfig->slaveReactors();
    for (SlaveReactor::Ptr &slaveReactor: slaveReactors)
    {
        slaveReactor->start();
    }
    LOG->write(utilities::LogType::Log_Info, FILE_INFO, "start all slave reactor successfully");
    // 心跳检查器
    m_serviceConfig->clientAliveChecker()->start();
    LOG->write(utilities::LogType::Log_Info, FILE_INFO, "start client alive checker successfully");
    // Session派发器
    m_serviceConfig->sessionDispatcher()->start();
    LOG->write(utilities::LogType::Log_Info, FILE_INFO, "start session dispatcher successfully");
    // Session销毁器
    m_serviceConfig->sessionDestroyer()->start();
    LOG->write(utilities::LogType::Log_Info, FILE_INFO, "start session destroyer successfully");
    // 如果还启动服务端
    if (false == m_serviceConfig->nodeConfig()->startAsClient())
    {
        // 启动主reactor的acceptor
        m_serviceConfig->acceptor()->start();
        LOG->write(utilities::LogType::Log_Info, FILE_INFO, "start acceptor successfully");
        m_serviceConfig->listenner()->start();
        LOG->write(utilities::LogType::Log_Info, FILE_INFO, "start listenner successfully");
    }
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

    return 0;
}

int TcpService::stop()
{
    m_serviceConfig->clientAliveChecker()->stop();
    LOG->write(utilities::LogType::Log_Info, FILE_INFO, "stop client heartbeat checker successfully");

    m_serviceConfig->hostsHeartbeatService()->stop();
    LOG->write(utilities::LogType::Log_Info, FILE_INFO, "stop hosts heartbeat service successfully");
    m_serviceConfig->hostsConnector()->stop();
    LOG->write(utilities::LogType::Log_Info, FILE_INFO, "stop hosts connector successfully");
    m_serviceConfig->packetProcessor()->stop();
    LOG->write(utilities::LogType::Log_Info, FILE_INFO, "stop packet processor successfully");
    if (false == m_serviceConfig->nodeConfig()->startAsClient())
    {
        m_serviceConfig->listenner()->stop();
        LOG->write(utilities::LogType::Log_Info, FILE_INFO, "stop listenner successfully");
        m_serviceConfig->acceptor()->stop();
        LOG->write(utilities::LogType::Log_Info, FILE_INFO, "stop acceptor successfully");
    }
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

    return 0;
}

void TcpService::registerModulePacketHandler(const std::int32_t moduleId,
                                             std::function<int(std::shared_ptr<std::vector<char>>)> packetHander)
{
    m_modulePacketHandler[moduleId] = std::move(packetHander);
}

bool TcpService::waitAtLeastOneNodeConnected(const int timeout)
{
    return m_serviceConfig->hostsInfoManager()->waitAtLeastOneNodeConnected(timeout);
}

int TcpService::boardcastModuleMessage(const std::int32_t moduleId, std::shared_ptr<std::vector<char>> data)
{
    PacketHeader packetHeader;
    packetHeader.setType(PacketType::PT_ModuleMessage);
    packetHeader.setModuleId(moduleId);
    packetHeader.setPayloadLength(data->size());

    // 编码包为待发送数据
    std::vector<char> buffer;
    buffer.resize(packetHeader.headerLength() + data->size());
    packetHeader.encode(buffer.data(), packetHeader.headerLength());
    memcpy(buffer.data() + packetHeader.headerLength(), data->data(), data->size());

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

int TcpService::sendModuleMessageByNodeId(const std::string &nodeId, const std::int32_t moduleId,
                                          std::shared_ptr<std::vector<char>> data)
{
    PacketHeader packetHeader;
    packetHeader.setType(PacketType::PT_ModuleMessage);
    packetHeader.setModuleId(moduleId);
    packetHeader.setPayloadLength(data->size());

    // 编码包为待发送数据
    std::vector<char> buffer;
    buffer.resize(packetHeader.headerLength() + data->size());
    packetHeader.encode(buffer.data(), packetHeader.headerLength());
    memcpy(buffer.data() + packetHeader.headerLength(), data->data(), data->size());

    // 发送给所有在线的节点
    int fd = m_serviceConfig->hostsInfoManager()->getHostFdById(nodeId);
    if (-1 == fd)
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


int TcpService::initServer()
{
    if (true == m_serviceConfig->nodeConfig()->startAsClient())
    {
        return 0;
    }

    // 创建监听套接字
    m_fd = utilities::Socket::create();
    if (-1 == m_fd)
    {
        LOG->write(utilities::LogType::Log_Error, FILE_INFO, "create socket failed, errno: ", errno, ", ", strerror(errno));
        return -1;
    }
    LOG->write(utilities::LogType::Log_Info, FILE_INFO, "create socket successfully, fd: ", m_fd);

#if 0   // 对于没有边缘触发的select来说，设置监听套接字非阻塞好像用处不大
    // 设置为非阻塞模式
        if(-1 == utilities::Socket::setNonBlock(m_fd))
        {
            return -1;
        }
#endif

    // 绑定监听地址端口
    if (-1 == utilities::Socket::bind(m_fd, m_serviceConfig->nodeConfig()->p2pIp(), m_serviceConfig->nodeConfig()->p2pPort()))
    {
        LOG->write(utilities::LogType::Log_Error, FILE_INFO, "bind socket failed, errno: ", errno, ", ", strerror(errno));
        utilities::Socket::close(m_fd);
        return -1;
    }
    LOG->write(utilities::LogType::Log_Info, FILE_INFO, "bind socket successfully");

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

    if (-1 == m_serviceConfig->listenner()->init(m_fd))
    {
        LOG->write(utilities::LogType::Log_Error, FILE_INFO, "select listenner init failed");
        utilities::Socket::close(m_fd);
        return -1;
    }
    LOG->write(utilities::LogType::Log_Info, FILE_INFO, "select listenner init successfully");

    // 完成一个客户端的创建，将客户端分配到从reactor进行recv/send处理
    m_serviceConfig->acceptor()->setNewClientCallback([this](const int fd, TcpSession::Ptr tcpSession) {
        // 将fd添加到TcpSession派发器中，由TcpSessionManager决定TcpSession由哪个SlaveReactor管理
        m_serviceConfig->tcpSessionDispatcher()->addSession(fd, [this, fd, tcpSession](const int slaveReactorIndex){
            if(-1 == m_serviceConfig->slaveReactors()[slaveReactorIndex]->addClient(fd))
            {
                LOG->write(utilities::LogType::Log_Error, FILE_INFO, "add client to SlaveReactor failed, fd: ", fd);
                return;
            }
            // 将fd和TcpSession添加到TcpSessionManager中
            m_serviceConfig->tcpSessionManager()->addTcpSession(fd, tcpSession);
            // 将fd添加到心跳检查器中
            m_serviceConfig->clientAliveChecker()->addClient(fd);
        });
    });

    // 注册新客户端上线回调
    m_serviceConfig->listenner()->registerConnectHandler([this]() { m_serviceConfig->acceptor()->onConnect(); });

    return 0;
}

int TcpService::initClient()
{
    if (-1 == m_serviceConfig->hostsInfoManager()->init(m_serviceConfig->nodeConfig()->nodesFile()))
    {
        LOG->write(utilities::LogType::Log_Error, FILE_INFO, "HostsInfoManager init failed");
        return -1;
    }
    LOG->write(utilities::LogType::Log_Info, FILE_INFO, "HostInfoManager init successfully");

    if (-1 == m_serviceConfig->hostsConnector()->init(m_serviceConfig->hostsInfoManager()))
    {
        LOG->write(utilities::LogType::Log_Error, FILE_INFO, "HostsConnector init failed");
        return -1;
    }
    LOG->write(utilities::LogType::Log_Info, FILE_INFO, "HostsConnector init successfully");

    if (-1 == m_serviceConfig->hostsHeartbeatService()->init(m_serviceConfig->nodeConfig()->id(),
                                                             m_serviceConfig->hostsInfoManager()))
    {
        LOG->write(utilities::LogType::Log_Error, FILE_INFO, "HostsHeartbeatService init failed");
        return -1;
    }
    LOG->write(utilities::LogType::Log_Info, FILE_INFO, "HostsHeartbeatService init successfully");

    m_serviceConfig->hostsConnector()->registerConnectHandler([this](const int fd, TcpSession::Ptr tcpSession) {
        LOG->write(utilities::LogType::Log_Info, FILE_INFO,
                   "connect ", tcpSession->peerHostEndPointInfo().host(), " successfully, dispatch TcpSession to SlaveReactor");

        std::string uuid = utilities::UUIDTool::generate();
        tcpSession->setHandshakeUuid(uuid);

        // 因为SlaveReactorManager分配TcpSession到各个SlaveReactor是异步的，所以这里要等待分配完成后发送ClientInfo包
        std::promise<std::size_t> addTcpSessionPromise;
        m_serviceConfig->sessionDispatcher()->addSession(fd, [this, tcpSession, &addTcpSessionPromise](const int slaveReactorIndex) {
            addTcpSessionPromise.set_value(slaveReactorIndex);
        });
        std::size_t slaveReactorIndex = addTcpSessionPromise.get_future().get();

        if(-1 == m_serviceConfig->slaveReactors()[slaveReactorIndex]->addClient(fd))
        {
            LOG->write(utilities::LogType::Log_Error, FILE_INFO, "add session to SlaveReactor, fd: ", fd);
            return;
        }

        m_serviceConfig->tcpSessionManager()->addTcpSession(fd, tcpSession);

        LOG->write(utilities::LogType::Log_Info, FILE_INFO, "send ClientInfo packet");

        // 发送ClientInfo包
        PacketClientInfo clientInfoPacket;
        clientInfoPacket.setLocalHost(m_serviceConfig->nodeConfig()->p2pIp() + ":" +
                                      csm::utilities::convertToString(m_serviceConfig->nodeConfig()->p2pPort()));
        clientInfoPacket.setPeerHost(tcpSession->peerHostEndPointInfo().host());
        clientInfoPacket.setHandshakeUuid(uuid);
        clientInfoPacket.setNodeId(m_serviceConfig->nodeConfig()->id());
        int payloadLength = clientInfoPacket.packetLength();

        PacketHeader packetHeader;
        packetHeader.setType(PacketType::PT_ClientInfo);
        packetHeader.setPayloadLength(clientInfoPacket.packetLength());
        int headerLength = packetHeader.headerLength();

        std::vector<char> buffer;
        buffer.resize(headerLength + payloadLength);
        packetHeader.encode(buffer.data(), headerLength);
        clientInfoPacket.encode(buffer.data() + headerLength, payloadLength);

        int ret = sendData(fd, buffer);
        LOG->write(utilities::LogType::Log_Info, FILE_INFO, "send ClientInfo ret: ", ret);
    });

    // 客户端发送ClientInfo包表明自己的身份后，将id与fd绑定，一方面供判定重复连接使用，一方面发送数据的时候通过id查找fd
    m_serviceConfig->slaveReactorManager()->registerClientInfoHandler([this](const int fd, TcpSession::Ptr tcpSession, PacketClientInfo::Ptr packetClientInfo){
        // 收到ClientInfo包，是别人连自己的情况
        std::string id = packetClientInfo->nodeId();
        tcpSession->setClientId(id);
        tcpSession->setHandshakeUuid(packetClientInfo->handshakeUuid());
        tcpSession->setClientOnlineTimestamp(utilities::Timestamp::getCurrentTimestamp());

        HostEndPointInfo localHostEndPointInfo(packetClientInfo->localHost());
        HostEndPointInfo peerHostEndPointInfo(packetClientInfo->peerHost());

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
        else if (true == m_serviceConfig->hostsInfoManager()->isHostIdExist(id))
        {
            LOG->write(utilities::LogType::Log_Error, FILE_INFO,
                       "host already connected, return -3 then SlaveReactor will disconnect it, this is who connect me: ", localHostEndPointInfo.host());
            replyResult = -3;
        }
        else
        {
            int ret = m_serviceConfig->hostsInfoManager()->addHostIdInfo(id, fd, packetClientInfo->handshakeUuid());
            if (-1 == ret)
            {
                LOG->write(utilities::LogType::Log_Error, FILE_INFO, "client already in HostInfoManager");
                replyResult = -1;
            }

            LOG->write(utilities::LogType::Log_Info, FILE_INFO, "add host id info to HostsInfoManager successfully, fd: ", fd, ", id: ", fd);
        }

        // 构造回应包
        PacketClientInfoReply reply;
        reply.setPeerHost(peerHostEndPointInfo.host());
        reply.setHandshakeUuid(packetClientInfo->handshakeUuid());
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
            LOG->write(utilities::LogType::Log_Error, FILE_INFO, "send ClientInfoReply packet failed", ", fd: ", fd, ", id: ", id);
            return -1;
        }

        LOG->write(utilities::LogType::Log_Info, FILE_INFO,
                   "send ClientInfoReply packet successfully, result:", replyResult, ", fd: ", fd, ", id: ", id);

        return 0;
    });

    // 收到ClientInfoReply包
    m_serviceConfig->slaveReactorManager()->registerClientInfoReplyHandler([this](const int fd, TcpSession::Ptr tcpSession, PacketClientInfoReply::Ptr packetClientInfoReply) -> int {
        tcpSession->setClientId(packetClientInfoReply->nodeId());
        tcpSession->setClientOnlineTimestamp(utilities::Timestamp::getCurrentTimestamp());

        HostEndPointInfo hostEndPointInfo(tcpSession->peerHostEndPointInfo());
        std::string id = packetClientInfoReply->nodeId();
        int result = packetClientInfoReply->result();

        LOG->write(utilities::LogType::Log_Info, FILE_INFO, "recv ClientInfoReply, host: ", hostEndPointInfo.host(), ", result: ", result);

        // 设置Host的id信息
        if (-1 == m_serviceConfig->hostsInfoManager()->setHostId(hostEndPointInfo, id))
        {
            LOG->write(utilities::LogType::Log_Error, FILE_INFO, "set host id failed, host: ", hostEndPointInfo.host());
            return -1;
        }
        LOG->write(utilities::LogType::Log_Info, FILE_INFO, "set host id successfully, host: ", hostEndPointInfo.host(), ", id: ", id);

        // 告知HostsConnector已经连接上，HostsConnector::setHostConnected内部操作将客户端从正在连接队列中移除
        if (-1 == m_serviceConfig->hostsConnector()->setHostConnected(hostEndPointInfo))
        {
            LOG->write(utilities::LogType::Log_Error, FILE_INFO, "set host connected failed, host: ", hostEndPointInfo.host());
            return -1;
        }
        LOG->write(utilities::LogType::Log_Info, FILE_INFO, "set host connected successfully, host: ", hostEndPointInfo.host());

        int anotherConnectionFd{ -1 };
        if (0 != result)
        {
            LOG->write(utilities::LogType::Log_Error, FILE_INFO, "ClientInfoReply result: ", result);
            if (-3 == result)
            {
                anotherConnectionFd = m_serviceConfig->hostsInfoManager()->getHostFdById(id);
                LOG->write(utilities::LogType::Log_Error, FILE_INFO,"get another connection fd: ", anotherConnectionFd);
                assert(-1 != anotherConnectionFd);

                TcpSession::Ptr anotherTcpSession = m_serviceConfig->tcpSessionManager()->tcpSession(anotherConnectionFd);
                assert(nullptr != anotherTcpSession);
                anotherTcpSession->setPeerHostEndPointInfo(packetClientInfoReply->peerHost());

                LOG->write(utilities::LogType::Log_Error, FILE_INFO,
                           "another connection fd: ", anotherConnectionFd, ", peer host: ", anotherTcpSession->peerHostEndPointInfo().host());
            }
            m_serviceConfig->sessionDestroyer()->addSession(fd, packetClientInfoReply->result());
            return -1;
        }

        // 可能是双方互相进行连接，但是对方已经发来了ClientInfo包，断开对方连自己的连接，仅保留自己连对方的连接
        std::string anotherConnectionUuid;
        if (true == m_serviceConfig->hostsInfoManager()->isHostIdExist(id, anotherConnectionFd, anotherConnectionUuid))
        {
            if (anotherConnectionUuid < packetClientInfoReply->handshakeUuid())
            {
                // 构造回应包
                PacketClientInfoReply reply;
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
                               "send ClientInfoReply packet failed", ", fd: ", anotherConnectionFd, ", id: ", id);
                    return -1;
                }

                LOG->write(utilities::LogType::Log_Info, FILE_INFO,
                           "send ClientInfoReply packet successfully", ", fd: ", anotherConnectionFd, ", id: ", id);
            }
            else if (anotherConnectionUuid > packetClientInfoReply->handshakeUuid())
            {
                return -1;
            }
            else
            {
                // 构造回应包
                PacketClientInfoReply reply;
                reply.setHandshakeUuid(anotherConnectionUuid);
                reply.setNodeId(m_serviceConfig->nodeConfig()->id());
                reply.setResult(-1);

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
                               "send ClientInfoReply packet failed", ", fd: ", anotherConnectionFd, ", id: ", id);
                    return -1;
                }

                LOG->write(utilities::LogType::Log_Info, FILE_INFO,
                           "send ClientInfoReply packet successfully", ", fd: ", anotherConnectionFd, ", id: ", id);
            }
        }
        else
        {
            /*
             * id是对方的id
             * fd是对端的连接套接字
             * uuid是我方连接对方的uuid
             */
            if (-1 == m_serviceConfig->hostsInfoManager()->addHostIdInfo(id, fd, packetClientInfoReply->handshakeUuid()))
            {
                LOG->write(utilities::LogType::Log_Error, FILE_INFO, "add id fd relation failed");
            }
        }

        return 0;
    });

    // 注册心跳发送回调
    m_serviceConfig->hostsHeartbeatService()->registerHeartbeatSender([this](const int fd, const std::vector<char> &data) {
        // 心跳发送失败就不管，如果是业务发送数据量过大，把缓冲区占满了，那服务端解业务包的时候也会刷新时间戳
        return sendData(fd, data);
    });

    return 0;
}

int TcpService::uninitServer()
{
    if (false == m_serviceConfig->nodeConfig()->startAsClient())
    {
        if (-1 == m_serviceConfig->acceptor()->uninit())
        {
            LOG->write(utilities::LogType::Log_Error, FILE_INFO,
                       "uninit Acceptor failed");
            return -1;
        }
        LOG->write(utilities::LogType::Log_Info, FILE_INFO,
                   "uninit Acceptor successfully");

        if (-1 == m_serviceConfig->listenner()->uninit())
        {
            LOG->write(utilities::LogType::Log_Error, FILE_INFO,
                       "uninit SelectListenner failed");
            return -1;
        }
        LOG->write(utilities::LogType::Log_Info, FILE_INFO,
                   "uninit SelectListenner successfully");

        // 关闭套接字
        if (-1 == utilities::Socket::close(m_fd))
        {
            LOG->write(utilities::LogType::Log_Error, FILE_INFO, "close fd failed");
            return -1;
        }
        LOG->write(utilities::LogType::Log_Info, FILE_INFO, "close socket successfully");
    }

    return 0;
}

int TcpService::uninitClient()
{
    if (-1 == m_serviceConfig->hostsHeartbeatService()->uninit())
    {
        LOG->write(utilities::LogType::Log_Error, FILE_INFO, "HostsHeartbeatService uninit failed");
        return -1;
    }
    LOG->write(utilities::LogType::Log_Info, FILE_INFO, "HostsHeartbeatService uninit successfully");

    if (-1 == m_serviceConfig->hostsConnector()->uninit())
    {
        LOG->write(utilities::LogType::Log_Error, FILE_INFO, "HostsConnector uninit failed");
        return -1;
    }
    LOG->write(utilities::LogType::Log_Info, FILE_INFO, "HostsConnector uninit successfully");

    if (-1 == m_serviceConfig->hostsInfoManager()->uninit())
    {
        LOG->write(utilities::LogType::Log_Error, FILE_INFO, "HostsInfoManager uninit failed");
        return -1;
    }
    LOG->write(utilities::LogType::Log_Info, FILE_INFO, "HostsInfoManager uninit successfully");

    return 0;
}

SlaveReactor::Ptr TcpService::getSlaveReactorByFd(const int fd)
{
    int slaveReactorIndex = m_serviceConfig->sessionDispatcher()->getSlaveReactorIndexByFd(fd);
    if(-1 == slaveReactorIndex)
    {
        LOG->write(utilities::LogType::Log_Error, FILE_INFO, "not found fd: ", fd);
        return nullptr;
    }

    return m_serviceConfig->slaveReactors()[slaveReactorIndex];
}

int TcpService::sendData(const int fd, const std::vector<char> &data)
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

int TcpService::disconnectClient(const HostEndPointInfo &hostEndPointInfo, const std::string &id, const std::string &uuid, const int flag)
{
    LOG->write(utilities::LogType::Log_Info, FILE_INFO, "id: ", id, ", flag: ", flag, ", hostEndPointInfo: ", hostEndPointInfo.host());
    if (0 == flag || -1 == flag)
    {
        if (-1 == m_serviceConfig->hostsInfoManager()->removeHostIdInfo(id, uuid))
        {
            LOG->write(utilities::LogType::Log_Info, FILE_INFO, "remove host id info failed, id: ", id);
        }
        else
        {
            LOG->write(utilities::LogType::Log_Info, FILE_INFO, "remove host id info successfully, id: ", id);
        }

        if (id == m_serviceConfig->nodeConfig()->id())
        {
            return 0;
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