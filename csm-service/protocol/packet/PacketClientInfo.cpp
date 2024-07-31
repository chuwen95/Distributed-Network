//
// Created by ChuWen on 9/16/23.
//

#include "PacketClientInfo.h"

using namespace csm::service;

PacketClientInfo::PacketClientInfo(std::shared_ptr<std::vector<char>> data)
{
    decode(data->data(), data->size());
}

std::size_t PacketClientInfo::packetLength() const
{
    return m_protoClientInfo.ByteSizeLong();
}

int PacketClientInfo::setSeq(const std::uint32_t seq)
{
    m_protoClientInfo.set_seq(seq);
    return 0;
}

std::uint32_t PacketClientInfo::seq()
{
    return m_protoClientInfo.seq();
}

int PacketClientInfo::setLocalHost(const std::string &host)
{
    m_protoClientInfo.set_localhost(host);
    return 0;
}

std::string PacketClientInfo::localHost()
{
    return m_protoClientInfo.localhost();
}

int PacketClientInfo::setPeerHost(const std::string &host)
{
    m_protoClientInfo.set_peerhost(host);
    return 0;
}

std::string PacketClientInfo::peerHost()
{
    return m_protoClientInfo.peerhost();
}

int PacketClientInfo::setHandshakeUuid(const std::string &uuid)
{
    m_protoClientInfo.set_handshakeuuid(uuid);
    return 0;
}

std::string PacketClientInfo::handshakeUuid()
{
    return m_protoClientInfo.handshakeuuid();
}

int PacketClientInfo::setNodeId(const std::string &nodeId)
{
    m_protoClientInfo.set_nodeid(std::string(nodeId));

    return 0;
}

std::string PacketClientInfo::nodeId()
{
    return m_protoClientInfo.nodeid();
}

int PacketClientInfo::encode(char *buffer, const std::size_t length) const
{
    m_protoClientInfo.SerializeToArray(buffer, length);

    return 0;
}

int PacketClientInfo::decode(const char *buffer, const std::size_t length)
{
    m_protoClientInfo.ParseFromArray(buffer, length);

    return 0;
}