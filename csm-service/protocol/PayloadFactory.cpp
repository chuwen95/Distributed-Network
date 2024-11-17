//
// Created by ChuWen on 2024/8/23.
//

#include "PayloadFactory.h"

#include "payload/PayloadClientInfo.h"
#include "payload/PayloadClientInfoReply.h"
#include "payload/PayloadHeartBeat.h"
#include "payload/PayloadHeartBeatReply.h"
#include "payload/PayloadModuleMessage.h"

using namespace csm::service;

PayloadBase::Ptr PayloadFactory::createPayload(const PacketType packetType)
{
    switch(packetType)
    {
        case PacketType::PT_ClientInfo:
            return std::make_shared<PayloadClientInfo>();
        case PacketType::PT_ClientInfoReply:
            return std::make_shared<PayloadClientInfoReply>();
        case PacketType::PT_HeartBeat:
            return std::make_shared<PayloadHeartBeat>();
        case PacketType::PT_HeartBeatReply:
            return std::make_shared<PayloadHeartBeatReply>();
        case PacketType::PT_ModuleMessage:
            return std::make_shared<PayloadModuleMessage>();
        default:
            return nullptr;
    }
}

PayloadBase::Ptr PayloadFactory::createPayload(const PacketType packetType, const char *data, const std::size_t len)
{
    PayloadBase::Ptr packet = createPayload(packetType);
    if(nullptr != packet)
    {
        packet->decode(data, len);
    }

    return packet;
}

