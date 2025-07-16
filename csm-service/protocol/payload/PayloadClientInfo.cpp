//
// Created by ChuWen on 9/16/23.
//

#include "PayloadClientInfo.h"

using namespace csm::service;

PayloadClientInfo::PayloadClientInfo(std::shared_ptr<std::vector<char>> data)
{
    m_protoPacket.ParseFromArray(data->data(), data->size());
}

void PayloadClientInfo::setSeq(const std::uint32_t seq)
{
    m_protoPacket.set_seq(seq);
}

std::uint32_t PayloadClientInfo::seq() const
{
    return m_protoPacket.seq();
}

void PayloadClientInfo::setLocalHost(const std::string &host)
{
    m_protoPacket.set_localhost(host);
}

std::string PayloadClientInfo::localHost() const
{
    return m_protoPacket.localhost();
}

void PayloadClientInfo::setPeerHost(const std::string &host)
{
    m_protoPacket.set_peerhost(host);
}

std::string PayloadClientInfo::peerHost() const
{
    return m_protoPacket.peerhost();
}

void PayloadClientInfo::setHandshakeUuid(const std::string &uuid)
{
    m_protoPacket.set_handshakeuuid(uuid);
}

std::string PayloadClientInfo::handshakeUuid() const
{
    return m_protoPacket.handshakeuuid();
}

void PayloadClientInfo::setNodeId(const std::string &nodeId)
{
    m_protoPacket.set_nodeid(nodeId);
}

std::string PayloadClientInfo::nodeId() const
{
    return m_protoPacket.nodeid();
}