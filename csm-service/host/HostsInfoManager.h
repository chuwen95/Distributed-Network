//
// Created by ChuWen on 9/22/23.
//

#ifndef TCPNETWORK_HOSTSINFOMANAGER_H
#define TCPNETWORK_HOSTSINFOMANAGER_H

#include "csm-common/Common.h"
#include "csm-framework/cluster/Common.h"

#include "HostEndPointInfo.h"
#include "csm-service/service/P2PSession.h"

namespace csm
{

    namespace service
    {

        class HostsInfoManager
        {
        public:
            explicit HostsInfoManager(std::string nodesFile);
            ~HostsInfoManager() = default;

            using Hosts =
                std::unordered_map<HostEndPointInfo, std::pair<NodeId, std::uint64_t>, HostEndPointInfo::hashFunction>;

        public:
            int init();

            auto getHosts() -> const Hosts&;

            /**
             * brief 设置host的id
             *
             * @param endPointInfo
             * @param id
             * @return
             */
            int setHostId(const HostEndPointInfo& endPointInfo, const NodeId& nodeId);

            /**
             * @brief 将id和fd对应起来
             *
             * @param nodeId
             * @param fd
             * @return
             */
            int addHostIdInfo(const NodeId& nodeId, SessionId sessionId);

            /**
             * @brief 将id和fd对应起来
             *
             * @param nodeId
             * @param fd
             * @return
             */
            int removeHostIdInfo(const NodeId& nodeId, SessionId sessionId);

            /**
             * @brief 清空Host的id，Host掉线的时候会调用该函数
             *
             * @param endPointInfo
             * @return
             */
            int setHostNotConnected(const HostEndPointInfo& endPointInfo);

            /**
             * brief 根据id得到fd
             *
             * @param nodeId
             * @param fd
             * @return
             */
            int getSessionId(const NodeId& nodeId, SessionId& sessionId);

            /**
             * brief 判断host是否在线
             *
             * @param id
             * @return
             */
            bool isHostIdExist(const NodeId& id);

            /**
             * brief 在线客户端数量
             *
             * @return
             */
            std::vector<csm::NodeId> onlineNodeIds();

            /**
             * brief 获取所有在线客户端
             *
             * @return
             */
            std::vector<std::pair<NodeId, SessionId>> getAllHosts();

        private:
            std::string m_nodesFile;

            // HostEndPointInfo => <Node ID, ClientInfoReply包时间戳>
            // 当对端回复ClientInfoReply后，second会有值，如果second有值，说明连上了
            std::mutex x_hosts;
            Hosts m_hosts;

            // Node ID => SessionId
            std::unordered_map<NodeId, SessionId> m_nodeIdInfos;
        };

    } // service

}

#endif //TCPNETWORK_HOSTSINFOMANAGER_H
