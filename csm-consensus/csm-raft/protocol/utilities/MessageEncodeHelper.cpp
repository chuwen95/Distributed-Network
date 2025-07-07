//
// Created by chu on 4/24/25.
//

#include "MessageEncodeHelper.h"

#include "csm-consensus/csm-raft/protocol/packet/RaftMessagePack.h"

using namespace csm::consensus;

int MessageEncodeHelper::encode(MessageBase::Ptr message, std::vector<char>& buffer)
{
    // 计算RequestVoteMsg序列化大小
    std::size_t requestVoteEncodeSize = message->encodeSize();

    // 最终发出的包
    RaftMessagePack raftMessage;
    raftMessage.setMessageType(RaftMessageType::RequestVote);
    raftMessage.setPayloadSize(requestVoteEncodeSize);

    // RequestVoteMsg序列化，缓冲区直接使用RaftMessage的payload
    message->encode(raftMessage.payloadRawPointer(), requestVoteEncodeSize);

    // RaftMessage序列化
    buffer.resize(raftMessage.encodeSize());
    raftMessage.encode(buffer.data(), raftMessage.encodeSize());

    return 0;
}