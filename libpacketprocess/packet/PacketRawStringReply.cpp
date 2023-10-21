//
// Created by root on 9/11/23.
//

#include "PacketRawStringReply.h"

namespace packetprocess
{

    std::size_t PacketRawStringReply::packetLength() const
    {
        return m_protoRawStringReply.ByteSizeLong();
    }

    std::string PacketRawStringReply::getResult()
    {
        return m_protoRawStringReply.result();
    }

    int PacketRawStringReply::setResult(const std::string &result)
    {
        m_protoRawStringReply.set_result(result);

        return 0;
    }

    int PacketRawStringReply::encode(char *buffer, const std::size_t length) const
    {
        m_protoRawStringReply.SerializeToArray(buffer, length);

        return 0;
    }

    int PacketRawStringReply::decode(const char *buffer, const std::size_t length)
    {
        m_protoRawStringReply.ParseFromArray(buffer, length);

        return 0;
    }

} // packetprocessor