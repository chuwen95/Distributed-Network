//
// Created by root on 9/16/23.
//

#include "PacketClientInfoReply.h"

namespace packetprocess
{

    PacketClientInfoReply::PacketClientInfoReply()
    {
        m_protoClientInfoReply.set_result(-1);
    }

    std::size_t PacketClientInfoReply::packetLength()
    {
        return m_protoClientInfoReply.ByteSizeLong();
    }

    int PacketClientInfoReply::setResult(const std::int32_t result)
    {
        m_protoClientInfoReply.set_result(result);

        return 0;
    }

    std::uint32_t PacketClientInfoReply::getResult()
    {
        return m_protoClientInfoReply.result();
    }

    int PacketClientInfoReply::encode(char *buffer, const std::size_t length)
    {
        m_protoClientInfoReply.SerializeToArray(buffer, length);

        return 0;
    }

    int PacketClientInfoReply::decode(const char *buffer, const std::size_t length)
    {
        m_protoClientInfoReply.ParseFromArray(buffer, length);

        return 0;
    }

} // packetprocess