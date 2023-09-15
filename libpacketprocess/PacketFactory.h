//
// Created by root on 9/11/23.
//

#ifndef TCPNETWORK_PACKETFACTORY_H
#define TCPNETWORK_PACKETFACTORY_H

#include "libcommon/Common.h"
#include "libcomponents/RingBuffer.h"
#include "packet/PacketBase.h"

namespace packetprocess
{

    class PacketFactory
    {
    public:
        PacketFactory();
        ~PacketFactory();

    public:
        /**
         * @brief 从环形缓冲区中取数据创建消息包
         *
         * @param type 包类型
         * @return
         */
        PacketBase::Ptr createPacket(const PacketType packetType, const std::shared_ptr<std::vector<char>>& data);
    };

} // packetprocess

#endif //TCPNETWORK_PACKETFACTORY_H
