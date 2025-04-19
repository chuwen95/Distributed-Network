//
// Created by ChuWen on 2024/6/12.
//

#include "ClusterPeerNode.h"

using namespace csm::consensus;

void ClusterPeerNode::setIsVote(const bool isVote)
{
    m_isVote = isVote;
}

bool ClusterPeerNode::isVote() const
{
    return m_isVote;
}