//
// Created by root on 9/11/23.
//

#include "PacketFactory.h"

#include "protocol/packet_header.pb.h"

#include "packet/PacketHeartBeat.h"
#include "packet/PacketHeartBeatReply.h"
#include "packet/PacketRawString.h"
#include "packet/PacketRawStringReply.h"

namespace packetprocess
{

    PacketFactory::PacketFactory()
    {}

    PacketFactory::~PacketFactory()
    {}

    PacketBase::Ptr PacketFactory::createPacket(const PacketType packetType, const std::shared_ptr<std::vector<char>>& data)
    {
        // 构造消息包
        PacketBase::Ptr packet;
        switch (packetType)
        {
            case packetprocess::PacketType::PT_HeartBeat:
                packet = std::make_shared<PacketHeartBeat>();
                break;
            case packetprocess::PacketType::PT_HeartBeatReply:
                packet = std::make_shared<PacketHeartBeatReply>();
                break;
            case packetprocess::PacketType::PT_RawString:
                packet = std::make_shared<PacketRawString>();
                break;
            case packetprocess::PacketType::PT_RawStringReply:
                packet = std::make_shared<PacketRawStringReply>();
                break;
            default:
                return nullptr;
        }

        // 解码消息包
        if(nullptr != data)
        {
            packet->decode(data->data(), data->size());
        }
        return packet;
    }

} // packetprocess