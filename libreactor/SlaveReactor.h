//
// Created by root on 9/5/23.
//

#ifndef TCPSERVER_SLAVEREACTOR_H
#define TCPSERVER_SLAVEREACTOR_H

#include "libcommon/Common.h"
#include "libcomponents/Thread.h"
#include "TcpSession.h"
#include "ClientAliveChecker.h"
#include "libpacketprocess/packet/PacketHeader.h"
#include "libpacketprocess/packet/PacketBase.h"

namespace server
{

    class SlaveReactor : public std::enable_shared_from_this<SlaveReactor>
    {
    public:
        using Ptr = std::shared_ptr<SlaveReactor>;

        SlaveReactor(const int id);
        ~SlaveReactor();

    public:
        int init();

        int uninit();

        int start();

        int stop();

        int addClient(TcpSession::Ptr tcpSession);

        std::size_t clientSize();

        void registerRecvHandler(std::function<void(const int, const packetprocess::PacketType, std::shared_ptr<std::vector<char>>&,
                                                    std::function<int(const int, const std::vector<char>&)>)>);

        void registerDisconnectHandler(std::function<void(const int id, const std::string&)> disconnectHandler);

        int sendData(const int fd, const char* data, const std::size_t size);

    private:
        void onClientDisconnect(const int fd);

        int getPacket(const int fd, components::RingBuffer::Ptr& readBuffer, packetprocess::PacketType& packetType, std::shared_ptr<std::vector<char>>& data);

        int processClientInfoPacket(const int fd, TcpSession::Ptr tcpSession, packetprocess::PacketBase::Ptr packet);

    private:
        // SlaveReactor的id
        int m_id;

        // clientfd => TcpSession
        std::mutex x_clientSessions;
        std::unordered_map<int, TcpSession::Ptr> m_fdSessions;

        components::Thread m_thread;

        int m_epfd{-1};

        // 数据接收回调
        std::function<void(const int fd, const packetprocess::PacketType, std::shared_ptr<std::vector<char>>&,
                           std::function<int(const int, const std::vector<char>&)>)> m_recvHandler;
        // 客户端断开回调
        std::function<void(const int id, const std::string&)> m_disconnectHandler;

        // 有EpollIn的事件回调
        std::unordered_set<int> m_infds;
        // fd收到了数据，需要处理
        std::unordered_set<int> m_datafds;
        // 若fd存在于m_outfds中，表明有数据要发送
        // 若m_outfds[fd] = true，表明正在等待EPOLLOUT事件
        std::mutex x_outfds;
        std::unordered_map<int, bool> m_outfds;

        // 客户端心跳检测
        ClientAliveChecker m_clientAliveChecker;

        // 多个线程同时写入待发送的数据，需要加锁
        std::mutex x_writeBuffer;
    };

}

#endif //TCPSERVER_SLAVEREACTOR_H