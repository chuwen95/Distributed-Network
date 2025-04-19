//
// Created by chu on 4/9/25.
//

#include "RaftMessagePack.h"

using namespace csm::consensus;

void RaftMessagePack::setMessageType(RaftMessageType type)
{
    m_rawRaftMessage.set_messagetype(type);
}

RaftMessageType RaftMessagePack::messageType() const
{
    return m_rawRaftMessage.messagetype();
}

void RaftMessagePack::setNodeIndex(const std::uint32_t nodeIndex)
{
    m_rawRaftMessage.set_nodeindex(nodeIndex);
}

std::uint32_t RaftMessagePack::nodeIndex() const
{
    return m_rawRaftMessage.nodeindex();
}

void RaftMessagePack::setPayloadSize(std::size_t size)
{
    m_rawRaftMessage.mutable_payload()->resize(size);
}

std::size_t RaftMessagePack::payloadSize() const
{
    return m_rawRaftMessage.payload().size();
}

char* RaftMessagePack::payloadRawPointer()
{
    return m_rawRaftMessage.mutable_payload()->data();
}

std::size_t RaftMessagePack::encodeSize() const
{
    return m_rawRaftMessage.ByteSizeLong();
}

void RaftMessagePack::encode(char* buffer, std::size_t size) const
{
    m_rawRaftMessage.SerializeToArray(buffer, size);
}

void RaftMessagePack::decode(char* buffer, std::size_t size)
{
    m_rawRaftMessage.ParseFromArray(buffer, size);
}
