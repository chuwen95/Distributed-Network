//
// Created by ChuWen on 9/16/23.
//

#include "PayloadClientInfo.h"

using namespace csm::service;

PayloadClientInfo::PayloadClientInfo(std::shared_ptr<std::vector<char>> data)
{
    decode(data->data(), data->size());
}

std::size_t PayloadClientInfo::packetLength() const
{
    return m_protoClientInfo.ByteSizeLong();
}

int PayloadClientInfo::setSeq(const std::uint32_t seq)
{
    m_protoClientInfo.set_seq(seq);
    return 0;
}

std::uint32_t PayloadClientInfo::seq()
{
    return m_protoClientInfo.seq();
}

int PayloadClientInfo::setLocalHost(const std::string &host)
{
    m_protoClientInfo.set_localhost(host);
    return 0;
}

std::string PayloadClientInfo::localHost()
{
    return m_protoClientInfo.localhost();
}

int PayloadClientInfo::setPeerHost(const std::string &host)
{
    m_protoClientInfo.set_peerhost(host);
    return 0;
}

std::string PayloadClientInfo::peerHost()
{
    return m_protoClientInfo.peerhost();
}

int PayloadClientInfo::setHandshakeUuid(const std::string &uuid)
{
    m_protoClientInfo.set_handshakeuuid(uuid);
    return 0;
}

std::string PayloadClientInfo::handshakeUuid()
{
    return m_protoClientInfo.handshakeuuid();
}

int PayloadClientInfo::setNodeId(const std::string &nodeId)
{
    m_protoClientInfo.set_nodeid(std::string(nodeId));

    return 0;
}

std::string PayloadClientInfo::nodeId()
{
    return m_protoClientInfo.nodeid();
}

int PayloadClientInfo::encode(char *buffer, const std::size_t length) const
{
    m_protoClientInfo.SerializeToArray(buffer, length);

    return 0;
}

int PayloadClientInfo::decode(const char *buffer, const std::size_t length)
{
    m_protoClientInfo.ParseFromArray(buffer, length);

    return 0;
}