//
// Created by ChuWen on 2024/6/12.
//

#include "ClusterLocalNode.h"

using namespace csm::consensus;

void ClusterLocalNode::setIsVote(const bool isVote)
{
    m_isVote = isVote;
}

bool ClusterLocalNode::isVote() const
{
    return m_isVote;
}