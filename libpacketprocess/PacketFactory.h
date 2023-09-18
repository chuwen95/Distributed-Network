//
// Created by root on 9/11/23.
//

#ifndef TCPNETWORK_PACKETFACTORY_H
#define TCPNETWORK_PACKETFACTORY_H

#include "libcommon/Common.h"
#include "libcomponents/RingBuffer.h"
#include "packet/PacketBase.h"
#include "packet/PacketReplyBase.h"

namespace packetprocess
{

    class PacketFactory
    {
    public:
        PacketFactory();
        ~PacketFactory();

    public:
        /**
         * @brief 创建消息包
         *
         * @param packetType 包类型
         * @return
         */
        PacketBase::Ptr createPacket(const PacketType packetType, const std::shared_ptr<std::vector<char>>& data);

        /**
         * @brief 根据消息包类型创建回应包
         *
         * @param packetType 包类型
         * @return
         */
        PacketReplyBase::Ptr createReplyPacket(const PacketType packetType);
    };

} // packetprocess

#endif //TCPNETWORK_PACKETFACTORY_H
