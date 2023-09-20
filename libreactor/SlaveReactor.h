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

        /**
         * @brief 参数主要是用于设置线程名
         * @param id
         */
        SlaveReactor(const int id);
        ~SlaveReactor();

    public:
        int init();

        int uninit();

        int start();

        int stop();

        /**
         * @brief 添加客户端
         *
         * @param tcpSession
         * @return
         */
        int addClient(TcpSession::Ptr tcpSession);

        /**
         * #brief 返回本SlaveReactor管理的客户端数量
         *
         * @return
         */
        std::size_t clientSize();

        /**
         * @brief 客户端有包需要处理的回调，回调包类型和包负载
         *
         * @param recvHandler
         */
        void registerRecvHandler(std::function<void(const int, const packetprocess::PacketType, std::shared_ptr<std::vector<char>>&,
                                                    std::function<int(const int, const std::vector<char>&)>)> recvHandler);

        /**
         * @brief 客户端离线回调
         *
         * @param disconnectHandler
         */
        void registerDisconnectHandler(std::function<void(const int id, const std::string&)> disconnectHandler);

        /**
         * @brief 写入待发送的数据到writeBuffer，如果writeBuffer的可用空间小于要写入的数据大小，则返回-1，客户端需重新尝试写入
         * 这里不能writeBuffer可用空间还剩多少就写入多少，防止多个客户端同时写入的时候包数据混乱
         *
         * @param fd 客户端fd
         * @param data 数据指针
         * @param size 数据大小
         * @return
         */
        int sendData(const int fd, const char* data, const std::size_t size);

        /**
         * @brief 获取客户端上线的时间戳，供业务线程池用，业务线程池放入任务的时候会记录放入任务那一刻的时间戳
         * 如果客户端不存在此函数返回-1，表示客户端已经离线，直接丢弃任务
         * 如果任务时间戳小于客户端上线时间戳，可能是客户端离线后又上线但是fd分配的是一样的
         *
         * @param fd 客户端fd
         * @return
         */
        std::uint32_t getClientOnlineTimestamp(const int fd);

    private:
        // 客户端离线处理
        void onClientDisconnect(const int fd);

        // 从readBuffer中获取包类型和包负载，如果缓冲区的数据不够包长度，则返回-1
        int getPacket(const int fd, components::RingBuffer::Ptr& readBuffer, packetprocess::PacketType& packetType, std::shared_ptr<std::vector<char>>& data);

        // 处理客户端ClientInfo包
        int processClientInfoPacket(const int fd, TcpSession::Ptr tcpSession, packetprocess::PacketBase::Ptr packet);

        // ClientAliveChecker调用，超时未收到心跳
        void onClientsHeartbeatTimeout(const std::vector<int>& fds);

    private:
        // SlaveReactor的id
        int m_id;

        // clientfd => TcpSession
        std::mutex x_clientSessions;
        std::unordered_map<int, TcpSession::Ptr> m_fdSessions;

        std::atomic_bool m_isTerminate{false};
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
        // 若m_outfds[fd].first = true，表明正在等待EPOLLOUT事件
        // 若m_outfds[fd].second = true，表示EPOLLOUT事件到来
        std::mutex x_outfds;
        std::unordered_map<int, std::pair<bool, bool>> m_outfds;

        // 客户端心跳检测
        ClientAliveChecker m_clientAliveChecker;

        // 多个线程同时写入待发送的数据，需要加锁
        std::mutex x_writeBuffer;
    };

}

#endif //TCPSERVER_SLAVEREACTOR_H