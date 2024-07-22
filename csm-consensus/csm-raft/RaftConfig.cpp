//
// Created by ChuWen on 2024/6/4.
//

#include "RaftConfig.h"

using namespace csm::consensus;

RaftConfig::RaftConfig(const std::string &id, const std::vector<std::string> &clusterServerIds) :
    m_id(id),
    m_clusterServerIds(clusterServerIds)
{ }

const std::string& RaftConfig::id()
{
    return m_id;
}

const std::vector<std::string>& RaftConfig::clusterServerIds() const
{
    return m_clusterServerIds;
}