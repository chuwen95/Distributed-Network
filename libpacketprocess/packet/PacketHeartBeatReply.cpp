//
// Created by root on 9/11/23.
//

#include "PacketHeartBeatReply.h"

namespace packetprocess
{

    std::size_t PacketHeartBeatReply::packetLength()
    {
        return m_protoHeartBeatReply.ByteSizeLong();
    }

    int PacketHeartBeatReply::encode(char *buffer, const std::size_t length)
    {
        m_protoHeartBeatReply.SerializeToArray(buffer, length);

        return 0;
    }

    int PacketHeartBeatReply::decode(const char *buffer, const std::size_t length)
    {
        m_protoHeartBeatReply.ParseFromArray(buffer, length);

        return 0;
    }

} // packetprocess