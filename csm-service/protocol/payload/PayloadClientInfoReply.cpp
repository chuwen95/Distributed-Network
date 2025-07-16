//
// Created by ChuWen on 9/16/23.
//

#include "PayloadClientInfoReply.h"

using namespace csm::service;

PayloadClientInfoReply::PayloadClientInfoReply(std::shared_ptr<std::vector<char>> data)
{
    m_protoPacket.ParseFromArray(data->data(), data->size());
}

void PayloadClientInfoReply::setSeq(const std::uint32_t seq)
{
    m_protoPacket.set_seq(seq);
}

std::uint32_t PayloadClientInfoReply::seq() const
{
    return m_protoPacket.seq();
}

void PayloadClientInfoReply::setPeerHost(const std::string &host)
{
    m_protoPacket.set_peerhost(host);
}

std::string PayloadClientInfoReply::peerHost() const
{
    return m_protoPacket.peerhost();
}

void PayloadClientInfoReply::setHandshakeUuid(const std::string &uuid)
{
    m_protoPacket.set_handshakeuuid(uuid);
}

std::string PayloadClientInfoReply::handshakeUuid() const
{
    return m_protoPacket.handshakeuuid();
}

void PayloadClientInfoReply::setNodeId(const std::string &id)
{
    m_protoPacket.set_nodeid(std::string(id));
}

std::string PayloadClientInfoReply::nodeId() const
{
    return m_protoPacket.nodeid();
}

void PayloadClientInfoReply::setResult(const std::int32_t result)
{
    m_protoPacket.set_result(result);
}

std::int32_t PayloadClientInfoReply::result() const
{
    return m_protoPacket.result();
}