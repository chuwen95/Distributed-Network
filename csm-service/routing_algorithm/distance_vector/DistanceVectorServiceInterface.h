//
// Created by ivy on 4/20/26.
//

#ifndef COPYSTATEMACHINE_DISTANCEVECTORSERVICEINTERFACE_H
#define COPYSTATEMACHINE_DISTANCEVECTORSERVICEINTERFACE_H

#include <optional>
#include <functional>

#include "csm-framework/cluster/Common.h"

#include "csm-service/protocol/header/PacketHeader.h"
#include "csm-service/protocol/payload/PayloadBase.h"

#include "DistanceVectorCommon.h"

namespace csm
{

    namespace service
    {

        class DistanceVectorServiceInterface
        {
        public:
            virtual ~DistanceVectorServiceInterface() = default;

        public:
            /**
             * @brief 启动距离向量服务
             */
            virtual void start() = 0;

            /**
             * @brief 停止距离向量服务
             */
            virtual void stop() = 0;

            /**
             * @brief 设置网络数据包发送器
             *
             * @param sender
             */
            virtual void setPacketSender(std::function<int(const NodeId& nodeId, const std::vector<char>& data)> sender) = 0;

            /**
             * @brief 处理网络数据包
             *
             * @param header
             * @param payload
             * @return
             */
            virtual int handlePacket(NodeId fromNodeId, PacketHeader::Ptr header, PayloadBase::Ptr payload) = 0;

            /**
             * @brief 查询目标节点路由
             *
             * @param targetNodeId
             * @return
             */
            virtual std::optional<std::pair<Distance, NodeId>> queryRoute(const NodeId& targetNodeId) = 0;
        };
    }
}

#endif //COPYSTATEMACHINE_DISTANCEVECTORSERVICEINTERFACE_H