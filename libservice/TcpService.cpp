//
// Created by root on 9/5/23.
//

#include "TcpService.h"
#include "libcomponents/Socket.h"
#include "libcomponents/Logger.h"
#include "libcomponents/UUIDTool.h"
#include "libpacketprocess/PacketFactory.h"
#include "libpacketprocess/packet/PacketClientInfo.h"
#include "libpacketprocess/packet/PacketClientInfoReply.h"

namespace service
{

    constexpr std::size_t c_conQueueSize{500};

    int TcpService::init(const std::string& config)
    {
        // 初始化服务配置
        if(-1 == m_serviceConfig.init(config))
        {
            return -1;
        }

        // 设置Logger
        if(-1 == components::Singleton<components::Logger>::instance()->init(m_serviceConfig.enableFileLog(), m_serviceConfig.logPath(), 4 * 1024 * 1024))
        {
            return -1;
        }
        components::Singleton<components::Logger>::instance()->setLogLevel(m_serviceConfig.logType());
        components::Singleton<components::Logger>::instance()->setConsoleOutput(m_serviceConfig.consoleOutput());

        // 是否作为客户端启动
        m_startAsClient = m_serviceConfig.startAsClient();

        if(-1 == m_slaveReactorManager.init(m_serviceConfig.slaveReactorNum(), m_serviceConfig.redispatchInterval(), m_serviceConfig.id()))
        {
            components::Singleton<components::Logger>::instance()->write(components::LogType::Log_Error, FILE_INFO, "slave reactor manager init failed");
            components::Socket::close(m_fd);
            return -1;
        }
        components::Singleton<components::Logger>::instance()->write(components::LogType::Log_Info, FILE_INFO, "slave reactor manager init successfully");

        // 初始化包处理线程池
        m_packetProcessThreadPoll.init(m_serviceConfig.packetProcessThreadNum(), "packet_proc");
        // 注册数据接收回调，SlaveReactor回调包类型和包负载二进制数据
        m_slaveReactorManager.registerRecvHandler([this](const int fd, packetprocess::PacketType packetType,
                                                         std::shared_ptr<std::vector<char>>& payloadData, const std::function<int(const int, const std::vector<char>&)>& writeHandler){
            std::uint32_t curTimestamp = components::CellTimestamp::getCurrentTimestamp();
            const auto expression = [fd, curTimestamp, packetType, payloadData, writeHandler, this]()
            {
                // 若任务时间戳小于客户端上线时间戳，任务直接返回不处理，因为可能是客户端离线后新的客户端被分配的相同的fd
                std::uint32_t onlineTimestamp = m_slaveReactorManager.getClientOnlineTimestamp(fd);
                if(0 == onlineTimestamp || curTimestamp < onlineTimestamp)
                {
                    return -1;
                }

                // 根据包类型和包负载构建请求包
                packetprocess::PacketFactory packetFactory;
                packetprocess::PacketBase::Ptr reqPacket = packetFactory.createPacket(packetType, payloadData);
                // 根据请求包类型构造回应包
                packetprocess::PacketType replyPacketType;
                packetprocess::PacketReplyBase::Ptr replyPacket = packetFactory.createReplyPacket(packetType, replyPacketType);
                if(nullptr != m_packetHandler)
                {
                    if(-1 == m_packetHandler(packetType, reqPacket, replyPacket))
                    {
                        components::Singleton<components::Logger>::instance()->write(components::LogType::Log_Info, FILE_INFO, "process packet failed");
                        return -1;
                    }
                }

                // 构造回应包数据
                std::size_t payloadLength = replyPacket->packetLength();
                std::vector<char> buffer;

                packetprocess::PacketHeader packetHeader;
                packetHeader.setType(replyPacketType);
                packetHeader.setPayloadLength(payloadLength);

                std::size_t headerLength = packetHeader.headerLength();
                std::size_t sumLength = headerLength + payloadLength;

                // 编码包
                buffer.resize(sumLength);
                packetHeader.encode(buffer.data(), headerLength);
                replyPacket->encode(buffer.data() + headerLength, payloadLength);

                if(nullptr != writeHandler)
                {
                    // 将回应包写入发送缓冲区
                    components::Singleton<components::Logger>::instance()->write(components::LogType::Log_Trace, FILE_INFO,
                                                                                 "try write reply packet to buffer, write size: ", sumLength, ", fd: ", fd);
                    int ret{0};
                    while(0 != (ret = writeHandler(fd, buffer)))
                    {
                        if(-2 == ret)
                        {
                            // 客户端已经离线
                            components::Singleton<components::Logger>::instance()->write(components::LogType::Log_Trace, FILE_INFO,
                                                                                         " client offline, fd: ", fd);
                            break;
                        }
                        else if(-1 == ret)
                        {
                            // 发送缓冲区满，等待10ms再次发送
                            std::this_thread::sleep_for(std::chrono::milliseconds(10));
                        }
                    }
                    components::Singleton<components::Logger>::instance()->write(components::LogType::Log_Trace, FILE_INFO,
                                                                                 " write reply packet to buffer successfully, write size: ", sumLength, ", fd: ", fd);
                }

                return 0;
            };
            m_packetProcessThreadPoll.push(expression);
        });

        // 注册客户端离线回调
        m_slaveReactorManager.registerDisconnectHandler(std::bind(&TcpService::onClientDisconnect, this, std::placeholders::_1,
                                                                  std::placeholders::_2, std::placeholders::_3, std::placeholders::_4));

        if(-1 == initServer())
        {
            components::Singleton<components::Logger>::instance()->write(components::LogType::Log_Error, FILE_INFO, "init server failed");
            return -1;
        }
        components::Singleton<components::Logger>::instance()->write(components::LogType::Log_Info, FILE_INFO, "init server successfully");

        if(-1 == initClient())
        {
            components::Singleton<components::Logger>::instance()->write(components::LogType::Log_Error, FILE_INFO, "init client failed");
            return -1;
        }
        components::Singleton<components::Logger>::instance()->write(components::LogType::Log_Info, FILE_INFO, "init client successfully");

        return 0;
    }

    int TcpService::uninit()
    {
        if(-1 == m_slaveReactorManager.uninit())
        {
            components::Singleton<components::Logger>::instance()->write(components::LogType::Log_Error, FILE_INFO, "uninit SlaveReactorManager failed");
            return -1;
        }
        components::Singleton<components::Logger>::instance()->write(components::LogType::Log_Info, FILE_INFO, "uninit SlaveReactorManager successfully");

        if(-1 == m_packetProcessThreadPoll.uninit())
        {
            components::Singleton<components::Logger>::instance()->write(components::LogType::Log_Error, FILE_INFO, "uninit PacketProcessThreadPool failed");
            return -1;
        }
        components::Singleton<components::Logger>::instance()->write(components::LogType::Log_Info, FILE_INFO, "uninit PacketProcessThreadPool successfully");

        if(-1 == uninitServer())
        {
            components::Singleton<components::Logger>::instance()->write(components::LogType::Log_Error, FILE_INFO, "uninit server failed");
            return -1;
        }
        components::Singleton<components::Logger>::instance()->write(components::LogType::Log_Info, FILE_INFO, "uninit server successfully");

        if(-1 == uninitClient())
        {
            components::Singleton<components::Logger>::instance()->write(components::LogType::Log_Error, FILE_INFO, "uninit client failed");
            return -1;
        }
        components::Singleton<components::Logger>::instance()->write(components::LogType::Log_Info, FILE_INFO, "uninit client successfully");

        components::Singleton<components::Logger>::instance()->uninit();

        return 0;
    }

    int TcpService::start()
    {
        components::Singleton<components::Logger>::instance()->start();

        // 包处理线程池
        m_packetProcessThreadPoll.start();
        // 从reactor管理器
        m_slaveReactorManager.start();
        // 如果还启动服务端
        if(false == m_startAsClient)
        {
            // 启动主reactor的acceptor
            m_acceptor.start();
            m_selectListenner.start();
        }
        /*
         * 本节点连接其他节点，对已经连上的节点发送心跳
         * 被连接的节点作为服务端，服务端不向客户端发送心跳
         */
        // 启动host心跳发送服务
        m_hostsHeartbeatService.start();
        // 启动host连接服务
        m_hostsConnector.start();

        return 0;
    }

    int TcpService::stop()
    {
        m_hostsHeartbeatService.stop();
        m_hostsConnector.stop();
        m_packetProcessThreadPoll.stop();
        if(false == m_startAsClient)
        {
            m_selectListenner.stop();
            m_acceptor.stop();
        }
        m_slaveReactorManager.stop();

        components::Singleton<components::Logger>::instance()->stop();

        return 0;
    }

    void TcpService::registerPacketHandler(std::function<int(const packetprocess::PacketType, packetprocess::PacketBase::Ptr,
                                                             packetprocess::PacketReplyBase::Ptr)> packetHandler)
    {
        m_packetHandler = std::move(packetHandler);
    }

    int TcpService::initServer()
    {
        if(true == m_startAsClient)
        {
            return 0;
        }

        m_ip = m_serviceConfig.ip();
        m_port = m_serviceConfig.port();

        // 创建监听套接字
        m_fd = components::Socket::create();
        if (-1 == m_fd)
        {
            components::Singleton<components::Logger>::instance()->write(components::LogType::Log_Error, FILE_INFO,
                                                                         "create socket failed, errno: ", errno,", ",strerror(errno));
            return -1;
        }
        components::Singleton<components::Logger>::instance()->write(components::LogType::Log_Info, FILE_INFO,
                                                                     "create socket successfully, fd: ", m_fd);

#if 0   // 对于没有边缘触发的select来说，设置监听套接字非阻塞好像用处不大
        // 设置为非阻塞模式
            if(-1 == components::Socket::setNonBlock(m_fd))
            {
                return -1;
            }
#endif

        // 绑定监听地址端口
        if (-1 == components::Socket::bind(m_fd, m_ip, m_port))
        {
            components::Singleton<components::Logger>::instance()->write(components::LogType::Log_Error, FILE_INFO,
                                                                         "bind socket failed, errno: ", errno, ", ",strerror(errno));
            components::Socket::close(m_fd);
            return -1;
        }
        components::Singleton<components::Logger>::instance()->write(components::LogType::Log_Info, FILE_INFO,
                                                                     "bind socket successfully");

        // 设置套接字为被动监听及连接队列长度
        if (-1 == components::Socket::listen(m_fd, c_conQueueSize))
        {
            components::Singleton<components::Logger>::instance()->write(components::LogType::Log_Error, FILE_INFO,
                                                                         "listen socket failed, errno: ", errno,", ",strerror(errno));
            components::Socket::close(m_fd);
            return -1;
        }
        components::Singleton<components::Logger>::instance()->write(components::LogType::Log_Info, FILE_INFO,
                                                                     "listen socket successfully");

        if (-1 == m_acceptor.init(m_fd))
        {
            components::Singleton<components::Logger>::instance()->write(components::LogType::Log_Error, FILE_INFO,
                                                                         "acceptor init failed");
            components::Socket::close(m_fd);
            return -1;
        }
        components::Singleton<components::Logger>::instance()->write(components::LogType::Log_Info, FILE_INFO,
                                                                     "acceptor init successfully");

        if (-1 == m_selectListenner.init(m_fd))
        {
            components::Singleton<components::Logger>::instance()->write(components::LogType::Log_Error, FILE_INFO,
                                                                         "select listenner init failed");
            components::Socket::close(m_fd);
            return -1;
        }
        components::Singleton<components::Logger>::instance()->write(components::LogType::Log_Info, FILE_INFO,
                                                                     "select listenner init successfully");

        // 完成一个客户端的创建，将客户端分配到从reactor进行recv/send处理
        m_acceptor.setNewClientCallback([this](const int fd, TcpSession::Ptr tcpSession) {
            m_slaveReactorManager.addTcpSession(tcpSession);
        });

        // 注册连接回调
        m_selectListenner.registerConnectHandler([this]() { m_acceptor.onConnect(); });

        return 0;
    }

    int TcpService::initClient()
    {
        m_hostsInfoManager = std::make_shared<HostsInfoManager>();
        if(-1 == m_hostsInfoManager->init(m_serviceConfig.nodesFile()))
        {
            components::Singleton<components::Logger>::instance()->write(components::LogType::Log_Error, FILE_INFO,
                                                                         "HostsInfoManager init failed");
            return -1;
        }
        components::Singleton<components::Logger>::instance()->write(components::LogType::Log_Info, FILE_INFO,
                                                                     "HostInfoManager init successfully");

        if(-1 == m_hostsConnector.init(m_hostsInfoManager))
        {
            components::Singleton<components::Logger>::instance()->write(components::LogType::Log_Error, FILE_INFO,
                                                                         "HostsConnector init failed");
            return -1;
        }
        components::Singleton<components::Logger>::instance()->write(components::LogType::Log_Info, FILE_INFO,
                                                                     "HostsConnector init successfully");

        if(-1 == m_hostsHeartbeatService.init(m_serviceConfig.id(), m_hostsInfoManager))
        {
            components::Singleton<components::Logger>::instance()->write(components::LogType::Log_Error, FILE_INFO,
                                                                         "HostsHeartbeatService init failed");
            return -1;
        }
        components::Singleton<components::Logger>::instance()->write(components::LogType::Log_Info, FILE_INFO,
                                                                     "HostsHeartbeatService init successfully");

        m_hostsConnector.registerConnectHandler([this](const int fd, TcpSession::Ptr tcpSession){

            components::Singleton<components::Logger>::instance()->write(components::LogType::Log_Info, FILE_INFO,
                                                                         "connect ", tcpSession->peerHostEndPointInfo().host(), " successfully, dispatch TcpSession to SlaveReactor");

            std::string uuid = components::UUIDTool::generate();
            tcpSession->setHandshakeUuid(uuid);

            // 因为SlaveReactorManager分配TcpSession到各个SlaveReactor是异步的，所以这里要等待分配完成后发送ClientInfo包
            std::promise<void> addTcpSessionPromise;
            m_slaveReactorManager.addTcpSession(tcpSession, [&addTcpSessionPromise](){
                addTcpSessionPromise.set_value();
            });

            addTcpSessionPromise.get_future().get();

            components::Singleton<components::Logger>::instance()->write(components::LogType::Log_Info, FILE_INFO,
                                                                         "send ClientInfo packet");

            // 发送ClientInfo包
            packetprocess::PacketClientInfo clientInfoPacket;
            clientInfoPacket.setLocalHost(m_serviceConfig.ip() + ":" + components::string_tools::convertToString(m_serviceConfig.port()));
            clientInfoPacket.setPeerHost(tcpSession->peerHostEndPointInfo().host());
            clientInfoPacket.setHandshakeUuid(uuid);
            clientInfoPacket.setNodeId(m_serviceConfig.id());
            int payloadLength = clientInfoPacket.packetLength();

            packetprocess::PacketHeader packetHeader;
            packetHeader.setType(packetprocess::PacketType::PT_ClientInfo);
            packetHeader.setPayloadLength(clientInfoPacket.packetLength());
            int headerLength = packetHeader.headerLength();

            std::vector<char> buffer;
            buffer.resize(headerLength + payloadLength);
            packetHeader.encode(buffer.data(), headerLength);
            clientInfoPacket.encode(buffer.data() + headerLength, payloadLength);

            int ret = m_slaveReactorManager.sendData(fd, buffer);
            components::Singleton<components::Logger>::instance()->write(components::LogType::Log_Info, FILE_INFO,
                                                                         "send ClientInfo ret: ", ret);
        });

        // 客户端发送ClientInfo包表明自己的身份后，将id与fd绑定，一方面供判定重复连接使用，一方面发送数据的时候通过id查找fd
        m_slaveReactorManager.registerClientInfoHandler([this](const HostEndPointInfo& localHostEndPointInfo,
                                                               const HostEndPointInfo& peerHostEndPointInfo, const int fd, const std::string& id, const std::string& uuid) -> int {
            // 收到ClientInfo包，是别人连自己的情况
            components::Singleton<components::Logger>::instance()->write(components::LogType::Log_Info, FILE_INFO,
                                                                         "recv ClientInfo, fd: ", fd, ", id: ", id);

            /*
             * -1: CommonError
             * -2: ConnectSelf
             * -3: DuplicateConnection
             */
            int replyResult{0};

            if(id == m_serviceConfig.id())
            {
                components::Singleton<components::Logger>::instance()->write(components::LogType::Log_Info, FILE_INFO,
                                                                             "connect self, set reply result to -2, then connection initiator will disconnect");
                replyResult = -2;
            }
            else if(true == m_hostsInfoManager->isHostIdExist(id))
            {
                components::Singleton<components::Logger>::instance()->write(components::LogType::Log_Error, FILE_INFO,
                                                                             "host already connected, return -3 then SlaveReactor will disconnect it, this is who connect me: ", localHostEndPointInfo.host());
                replyResult = -3;
            }
            else
            {
                int ret = m_hostsInfoManager->addHostIdInfo(id, fd, uuid);
                if(-1 == ret)
                {
                    components::Singleton<components::Logger>::instance()->write(components::LogType::Log_Error, FILE_INFO,
                                                                             "client already in HostInfoManager");
                    replyResult = -1;
                }

                components::Singleton<components::Logger>::instance()->write(components::LogType::Log_Info, FILE_INFO,
                                                                         "add host id info to HostsInfoManager successfully, fd: ", fd, ", id: ", fd);
            }

            // 构造回应包
            packetprocess::PacketClientInfoReply reply;
            reply.setPeerHost(peerHostEndPointInfo.host());
            reply.setHandshakeUuid(uuid);
            reply.setNodeId(m_serviceConfig.id());
            reply.setResult(replyResult);

            packetprocess::PacketHeader packetHeader;
            packetHeader.setType(packetprocess::PacketType::PT_ClientInfoReply);
            packetHeader.setPayloadLength(reply.packetLength());

            int headerLength = packetHeader.headerLength();
            int payloadLength = reply.packetLength();

            std::vector<char> buffer;
            buffer.resize(headerLength + reply.packetLength());

            packetHeader.encode(buffer.data(), headerLength);
            reply.encode(buffer.data() + headerLength, payloadLength);

            if(-1 == m_slaveReactorManager.sendData(fd, buffer))
            {
                components::Singleton<components::Logger>::instance()->write(components::LogType::Log_Error, FILE_INFO,
                                                                             "send ClientInfoReply packet failed", ", fd: ", fd, ", id: ", id);
                return -1;
            }

            components::Singleton<components::Logger>::instance()->write(components::LogType::Log_Info, FILE_INFO,
                                                                         "send ClientInfoReply packet successfully, result:", replyResult, ", fd: ", fd, ", id: ", id);

            return 0;
        });

        // 收到ClientInfoReply包
        m_slaveReactorManager.registerClientInfoReplyHandler([this](const HostEndPointInfo& hostEndPointInfo, const int fd, const std::string& id,
                const std::string& uuid, const int result) -> int {

            components::Singleton<components::Logger>::instance()->write(components::LogType::Log_Info, FILE_INFO,
                                                                         "recv ClientInfoReply, host: ", hostEndPointInfo.host(), ", result: ", result);

            // 设置Host的id信息
            if(-1 == m_hostsInfoManager->setHostId(hostEndPointInfo, id))
            {
                components::Singleton<components::Logger>::instance()->write(components::LogType::Log_Error, FILE_INFO,
                                                                             "set host id failed, host: ", hostEndPointInfo.host());
                return -1;
            }
            components::Singleton<components::Logger>::instance()->write(components::LogType::Log_Info, FILE_INFO,
                                                                         "set host id successfully, host: ", hostEndPointInfo.host(), ", id: ", id);

            // 告知HostsConnector已经连接上，HostsConnector::setHostConnected内部操作将客户端从正在连接队列中移除
            if(-1 == m_hostsConnector.setHostConnected(hostEndPointInfo))
            {
                components::Singleton<components::Logger>::instance()->write(components::LogType::Log_Error, FILE_INFO,
                                                                             "set host connected failed, host: ", hostEndPointInfo.host());
                return -1;
            }
            components::Singleton<components::Logger>::instance()->write(components::LogType::Log_Info, FILE_INFO,
                                                                         "set host connected successfully, host: ", hostEndPointInfo.host());


            if(0 != result)
            {
                components::Singleton<components::Logger>::instance()->write(components::LogType::Log_Error, FILE_INFO,
                                                                             "ClientInfoReply result: ", result);
                return -1;
            }

            // 可能是双方互相进行连接，但是对方已经发来了ClientInfo包，断开对方连自己的连接，仅保留自己连对方的连接
            int anotherConnectionFd;
            std::string anotherConnectionUuid;
            if(true == m_hostsInfoManager->isHostIdExist(id, anotherConnectionFd, anotherConnectionUuid))
            {
                if(anotherConnectionUuid <= uuid)
                {
                    // 构造回应包
                    packetprocess::PacketClientInfoReply reply;
                    reply.setHandshakeUuid(anotherConnectionUuid);
                    reply.setNodeId(m_serviceConfig.id());
                    reply.setResult(-3);

                    packetprocess::PacketHeader packetHeader;
                    packetHeader.setType(packetprocess::PacketType::PT_ClientInfoReply);
                    packetHeader.setPayloadLength(reply.packetLength());

                    int headerLength = packetHeader.headerLength();
                    int payloadLength = reply.packetLength();

                    std::vector<char> buffer;
                    buffer.resize(headerLength + reply.packetLength());

                    packetHeader.encode(buffer.data(), headerLength);
                    reply.encode(buffer.data() + headerLength, payloadLength);

                    if(-1 == m_slaveReactorManager.sendData(anotherConnectionFd, buffer))
                    {
                        components::Singleton<components::Logger>::instance()->write(components::LogType::Log_Error, FILE_INFO,
                                                                                     "send ClientInfoReply packet failed", ", fd: ", fd, ", id: ", id);
                        return -1;
                    }

                    components::Singleton<components::Logger>::instance()->write(components::LogType::Log_Info, FILE_INFO,
                                                                                 "send ClientInfoReply packet successfully", ", fd: ", fd, ", id: ", id);
                }
                else
                {
                    return -1;
                }
            }
            else
            {
                /*
                 * id是对方的id
                 * fd是对端的连接套接字
                 * uuid是我方连接对方的uuid
                 */
                if(-1 == m_hostsInfoManager->addHostIdInfo(id, fd, uuid))
                {
                    components::Singleton<components::Logger>::instance()->write(components::LogType::Log_Error, FILE_INFO,
                                                                                 "add id fd relation failed");
                }
            }

            return 0;
        });

        // 注册心跳发送回调
        m_hostsHeartbeatService.registerHeartbeatSender([this](const int fd, const std::vector<char>& data){
            // 心跳发送失败就不管，如果是业务发送数据量过大，把缓冲区占满了，那服务端解业务包的时候也会刷新时间戳
            return m_slaveReactorManager.sendData(fd, data);
        });

        return 0;
    }

    int TcpService::uninitServer()
    {
        if(false == m_startAsClient)
        {
            if (-1 == m_acceptor.uninit())
            {
                components::Singleton<components::Logger>::instance()->write(components::LogType::Log_Error, FILE_INFO,
                                                                             "uninit Acceptor failed");
                return -1;
            }
            components::Singleton<components::Logger>::instance()->write(components::LogType::Log_Info, FILE_INFO,
                                                                         "uninit Acceptor successfully");

            if (-1 == m_selectListenner.uninit())
            {
                components::Singleton<components::Logger>::instance()->write(components::LogType::Log_Error, FILE_INFO,
                                                                             "uninit SelectListenner failed");
                return -1;
            }
            components::Singleton<components::Logger>::instance()->write(components::LogType::Log_Info, FILE_INFO,
                                                                         "uninit SelectListenner successfully");

            // 关闭套接字
            if(-1 == components::Socket::close(m_fd))
            {
                components::Singleton<components::Logger>::instance()->write(components::LogType::Log_Error, FILE_INFO, "close fd failed");
                return -1;
            }
            components::Singleton<components::Logger>::instance()->write(components::LogType::Log_Info, FILE_INFO, "close socket successfully");
        }

        return 0;
    }

    int TcpService::uninitClient()
    {
        if(-1 == m_hostsHeartbeatService.uninit())
        {
            components::Singleton<components::Logger>::instance()->write(components::LogType::Log_Error, FILE_INFO, "HostsHeartbeatService uninit failed");
            return -1;
        }
        components::Singleton<components::Logger>::instance()->write(components::LogType::Log_Info, FILE_INFO, "HostsHeartbeatService uninit successfully");

        if(-1 == m_hostsConnector.uninit())
        {
            components::Singleton<components::Logger>::instance()->write(components::LogType::Log_Error, FILE_INFO, "HostsConnector uninit failed");
            return -1;
        }
        components::Singleton<components::Logger>::instance()->write(components::LogType::Log_Info, FILE_INFO, "HostsConnector uninit successfully");

        if(-1 == m_hostsInfoManager->uninit())
        {
            components::Singleton<components::Logger>::instance()->write(components::LogType::Log_Error, FILE_INFO, "HostsInfoManager uninit failed");
            return -1;
        }
        components::Singleton<components::Logger>::instance()->write(components::LogType::Log_Info, FILE_INFO, "HostsInfoManager uninit successfully");

        return 0;
    }

    int TcpService::onClientDisconnect(const HostEndPointInfo &hostEndPointInfo, const std::string& id, const std::string& uuid, const int flag)
    {
        components::Singleton<components::Logger>::instance()->write(components::LogType::Log_Info, FILE_INFO,
                                                                     "id: ", id, ", flag: ", flag);
        if(0 == flag || -1 == flag)
        {
            if(-1 == m_hostsInfoManager->removeHostIdInfo(id, uuid))
            {
                components::Singleton<components::Logger>::instance()->write(components::LogType::Log_Info, FILE_INFO,
                                                                             "remove host id info failed, id: ", id);
            }
            else
            {
                components::Singleton<components::Logger>::instance()->write(components::LogType::Log_Info, FILE_INFO,
                                                                             "remove host id info successfully, id: ", id);
            }

            if(id == m_serviceConfig.id())
            {
                return 0;
            }

            if(-1 == m_hostsInfoManager->setHostNotConnected(hostEndPointInfo))
            {
                components::Singleton<components::Logger>::instance()->write(components::LogType::Log_Info, FILE_INFO,
                                                                             "set host not connect failed: ", hostEndPointInfo.host());
            }
            else
            {
                components::Singleton<components::Logger>::instance()->write(components::LogType::Log_Info, FILE_INFO,
                                                                             "set host not connect successfully: ", hostEndPointInfo.host());
            }
        }
        else if(-2 == flag)
        {}
        else if(-3 == flag)
        {}

        return 0;
    }

}