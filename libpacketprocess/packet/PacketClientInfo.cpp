//
// Created by root on 9/16/23.
//

#include "PacketClientInfo.h"

namespace packetprocess
{

    std::size_t PacketClientInfo::packetLength()
    {
        return m_protoClientInfo.ByteSizeLong();
    }

    int PacketClientInfo::setId(const std::string_view id)
    {
        m_protoClientInfo.set_id(std::string(id));

        return 0;
    }

    std::string PacketClientInfo::getId()
    {
        return m_protoClientInfo.id();
    }

    int PacketClientInfo::encode(char *buffer, const std::size_t length)
    {
        m_protoClientInfo.SerializeToArray(buffer, length);

        return 0;
    }

    int PacketClientInfo::decode(const char *buffer, const std::size_t length)
    {
        m_protoClientInfo.ParseFromArray(buffer, length);

        return 0;
    }

} // packetprocess