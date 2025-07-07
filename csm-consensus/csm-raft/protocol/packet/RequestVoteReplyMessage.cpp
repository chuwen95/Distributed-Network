//
// Created by chu on 4/19/25.
//

#include "RequestVoteReplyMessage.h"

using namespace csm::consensus;

RequestVoteReplyMessage::RequestVoteReplyMessage() : MessageBase(RaftMessageType::RequestVoteReply)
{}

void RequestVoteReplyMessage::setTerm(const uint64_t term)
{
    m_rawRequestVoteReply.set_term(term);
}

std::uint64_t RequestVoteReplyMessage::term() const
{
    return m_rawRequestVoteReply.term();
}

void RequestVoteReplyMessage::setVoteGranted(const bool voteGranted)
{
    m_rawRequestVoteReply.set_votegranted(voteGranted);
}

bool RequestVoteReplyMessage::voteGranted() const
{
    return m_rawRequestVoteReply.votegranted();
}

std::size_t RequestVoteReplyMessage::encodeSize() const
{
    return m_rawRequestVoteReply.ByteSizeLong();
}

void RequestVoteReplyMessage::encode(char* buffer, std::size_t size) const
{
    m_rawRequestVoteReply.SerializeToArray(buffer, size);
}

void RequestVoteReplyMessage::decode(const char* buffer, std::size_t size)
{
    m_rawRequestVoteReply.ParseFromArray(buffer, size);
}