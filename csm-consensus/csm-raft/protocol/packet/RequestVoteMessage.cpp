//
// Created by chu on 4/9/25.
//

#include "RequestVoteMessage.h"

#include "csm-consensus/csm-raft/Raft.h"

using namespace csm::consensus;

RequestVoteMessage::RequestVoteMessage() : MessageBase(RaftMessageType::RequestVote)
{}

void RequestVoteMessage::setTerm(std::uint64_t term)
{
    m_rawRequestVote.set_term(term);
}

std::uint64_t RequestVoteMessage::term() const
{
    return m_rawRequestVote.term();
}

void RequestVoteMessage::setCandidateId(const std::uint32_t nodeId)
{
    m_rawRequestVote.set_candidateid(nodeId);
}

int RequestVoteMessage::candidateId() const
{
    return m_rawRequestVote.candidateid();
}

void RequestVoteMessage::setLastLogIndex(const std::uint64_t index)
{
    m_rawRequestVote.set_lastlogindex(index);
}

std::uint64_t RequestVoteMessage::lastLogIndex() const
{
    return m_rawRequestVote.lastlogindex();
}

void RequestVoteMessage::setLastLogTerm(const std::uint64_t term)
{
    m_rawRequestVote.set_lastlogterm(term);
}

std::uint64_t RequestVoteMessage::lastLogTerm() const
{
    return m_rawRequestVote.lastlogterm();
}

void RequestVoteMessage::encode(char* buffer, std::size_t size) const
{
    m_rawRequestVote.SerializeToArray(buffer, size);
}

std::size_t RequestVoteMessage::encodeSize() const
{
    return m_rawRequestVote.ByteSizeLong();
}

void RequestVoteMessage::decode(const char* buffer, std::size_t size)
{
    m_rawRequestVote.ParseFromArray(buffer, size);
}