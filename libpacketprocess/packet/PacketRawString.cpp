//
// Created by root on 9/11/23.
//

#include "PacketRawString.h"

namespace packetprocess
{

    std::size_t PacketRawString::packetLength() const
    {
        return m_protoRawString.ByteSizeLong();
    }

    int PacketRawString::setContent(const std::string &content)
    {
        m_protoRawString.set_content(content);

        return 0;
    }

    std::string PacketRawString::getContent()
    {
        return m_protoRawString.content();
    }

    int PacketRawString::encode(char *buffer, const std::size_t length) const
    {
        m_protoRawString.SerializeToArray(buffer, length);

        return 0;
    }

    int PacketRawString::decode(const char *buffer, const std::size_t length)
    {
        m_protoRawString.ParseFromArray(buffer, length);

        return 0;
    }

} // packetprocess