//
// Created by ChuWen on 9/22/23.
//

#ifndef TCPNETWORK_HOSTSINFOMANAGER_H
#define TCPNETWORK_HOSTSINFOMANAGER_H

#include "csm-common/Common.h"
#include "HostEndPointInfo.h"

namespace csm
{

    namespace service
    {

        class HostsInfoManager
        {
        public:
            using Ptr = std::shared_ptr<HostsInfoManager>;

            HostsInfoManager(const std::string& nodesFile);
            ~HostsInfoManager() = default;

            using Hosts = std::unordered_map<HostEndPointInfo, std::pair<std::string, std::uint64_t>, HostEndPointInfo::hashFunction>;

        public:
            int init();

            auto getHosts() -> const Hosts &;

            /**
             * brief 设置host的id
             *
             * @param endPointInfo
             * @param id
             * @return
             */
            int setHostId(const HostEndPointInfo &endPointInfo, const std::string &id);

            /**
             * @brief 将id和fd对应起来
             *
             * @param id
             * @param fd
             * @return
             */
            int addHostIdInfo(const std::string &id, const int fd, const std::string &uuid);

            /**
             * @brief 将id和fd对应起来
             *
             * @param id
             * @param fd
             * @return
             */
            int setHostIdInfo(const std::string &id, const int fd, const std::string &uuid);

            /**
             * @brief 将id和fd对应起来
             *
             * @param id
             * @param fd
             * @return
             */
            int removeHostIdInfo(const std::string &id, const std::string &uuid);

            /**
             * @brief 清空Host的id，Host掉线的时候会调用该函数
             *
             * @param endPointInfo
             * @return
             */
            int setHostNotConnected(const HostEndPointInfo &endPointInfo);

            /**
             * brief 根据id得到fd
             *
             * @param id
             * @return
             */
            int getHostFdById(const std::string id);

            /**
             * brief 判断host是否在线
             *
             * @param id
             * @param fd
             * @param uuid
             * @return
             */
            bool isHostIdExist(const std::string id, int &fd, std::string &uuid);

            /**
             * brief 判断host是否在线
             *
             * @param id
             * @return
             */
            bool isHostIdExist(const std::string id);

            /**
             * brief 在线客户端数量
             *
             * @return
             */
            std::uint32_t onlineClientSize();

            /**
             * brief 获取所有在线客户端
             *
             * @return
             */
            std::vector<std::pair<std::string, int>> getAllOnlineClients();

            /**
             * brief 等待至少一个节点连接成功
             * @param timeout       [in]超时（单位：毫秒）
             *
             * @return
             */
            bool waitAtLeastOneNodeConnected(const int timeout);

        private:
            std::string m_nodesFile;

            // HostEndPointInfo => <Client ID, ClientInfoReply包时间戳>
            // 当对端回复ClientInfoReply后，second会有值，如果second有值，说明连上了
            std::mutex x_hosts;
            Hosts m_hosts;

            // Client ID => fd, handshakeUuid
            std::unordered_map<std::string, std::pair<int, std::string>> m_nodeIdInfos;
        };

    } // service

}

#endif //TCPNETWORK_HOSTSINFOMANAGER_H
