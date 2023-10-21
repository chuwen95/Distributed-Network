//
// Created by root on 9/16/23.
//

#include "PacketClientInfoReply.h"

namespace packetprocess
{

    PacketClientInfoReply::PacketClientInfoReply()
    {
    }

    std::size_t PacketClientInfoReply::packetLength() const
    {
        return m_protoClientInfoReply.ByteSizeLong();
    }

    int PacketClientInfoReply::setSeq(const std::uint32_t seq)
    {
        m_protoClientInfoReply.set_seq(seq);
        return 0;
    }

    std::uint32_t PacketClientInfoReply::seq()
    {
        return m_protoClientInfoReply.seq();
    }

    int PacketClientInfoReply::setPeerHost(const std::string &host)
    {
        m_protoClientInfoReply.set_peerhost(host);
        return 0;
    }

    std::string PacketClientInfoReply::peerHost()
    {
        return m_protoClientInfoReply.peerhost();
    }

    int PacketClientInfoReply::setHandshakeUuid(const std::string &uuid)
    {
        m_protoClientInfoReply.set_handshakeuuid(uuid);
        return 0;
    }

    std::string PacketClientInfoReply::handshakeUuid()
    {
        return m_protoClientInfoReply.handshakeuuid();
    }

    int PacketClientInfoReply::setNodeId(const std::string& id)
    {
        m_protoClientInfoReply.set_nodeid(std::string(id));

        return 0;
    }

    std::string PacketClientInfoReply::nodeId()
    {
        return m_protoClientInfoReply.nodeid();
    }

    int PacketClientInfoReply::setResult(const std::int32_t result)
    {
        m_protoClientInfoReply.set_result(result);
        return 0;
    }

    std::int32_t PacketClientInfoReply::result()
    {
        return m_protoClientInfoReply.result();
    }

    int PacketClientInfoReply::encode(char *buffer, const std::size_t length) const
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