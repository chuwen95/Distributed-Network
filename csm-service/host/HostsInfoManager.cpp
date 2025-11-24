//
// Created by ChuWen on 9/22/23.
//

#include "HostsInfoManager.h"
#include "csm-utilities/ElapsedTime.h"
#include "csm-utilities/Logger.h"
#include "csm-utilities/StringTool.h"

#include <google/protobuf/message.h>
#include <json/json.h>

using namespace csm::service;

HostsInfoManager::HostsInfoManager(std::string nodesFile) : m_nodesFile(std::move(nodesFile))
{
}

int HostsInfoManager::init()
{
    std::ifstream nodesJsonFile(m_nodesFile, std::ios::in);
    if (false == nodesJsonFile.is_open())
    {
        LOG->write(utilities::LogType::Log_Error, FILE_INFO, "open nodes config file failed: ", m_nodesFile);
        return -1;
    }

    Json::Value root;
    Json::Reader reader;
    bool ret = reader.parse(nodesJsonFile, root);
    if (false == ret)
    {
        LOG->write(utilities::LogType::Log_Error, FILE_INFO, "parse nodes config file failed: ", m_nodesFile);
        return -1;
    }
    LOG->write(utilities::LogType::Log_Info, FILE_INFO, "parse nodes config file successfully: ", m_nodesFile);

    Json::Value& nodesValue = root["nodes"];
    if (false == nodesValue.isArray())
    {
        LOG->write(utilities::LogType::Log_Error, FILE_INFO, "parse hosts config file failed: ", m_nodesFile);
        return -1;
    }

    for (const Json::Value& value : nodesValue)
    {
        std::string host = value.asString();
        LOG->write(utilities::LogType::Log_Info, FILE_INFO, "hosts info: ", host);
        std::vector<std::string> ipPort = csm::utilities::split(host, ':');
        if (2 != ipPort.size())
        {
            LOG->write(utilities::LogType::Log_Error, FILE_INFO, "parse hosts ip and port failed: ", host);
            return -1;
        }

        const std::string& ip = ipPort[0];
        unsigned short port = csm::utilities::convertFromString<unsigned short>(ipPort[1]);
        m_hosts.emplace(HostEndPointInfo(ip, port), std::make_pair("", -1));
    }

    return 0;
}

auto HostsInfoManager::getHosts() -> const Hosts&
{
    std::unique_lock<std::mutex> ulock(x_hosts);
    return m_hosts;
}

int HostsInfoManager::setHostId(const HostEndPointInfo& endPointInfo, const NodeId& nodeId)
{
    std::unique_lock<std::mutex> ulock(x_hosts);
    auto iter = m_hosts.find(endPointInfo);
    if (m_hosts.end() == iter)
    {
        return -1;
    }
    iter->second.first = nodeId;
    return 0;
}

int HostsInfoManager::addHostIdInfo(const NodeId& nodeId, const SessionId sessionId)
{
    std::unique_lock<std::mutex> ulock(x_hosts);
    auto iter = m_nodeIdInfos.find(nodeId);
    if (m_nodeIdInfos.end() != iter)
    {
        LOG->write(utilities::LogType::Log_Info, FILE_INFO, "node already exist", nodeId, ", session id: ", sessionId);
        return -1;
    }

    m_nodeIdInfos.emplace(nodeId, sessionId);
    LOG->write(utilities::LogType::Log_Info, FILE_INFO, "node ", nodeId, ", add session id: ", sessionId);

    return 0;
}

int HostsInfoManager::removeHostIdInfo(const NodeId& nodeId, const SessionId sessionId)
{
    std::unique_lock<std::mutex> ulock(x_hosts);

    auto iter = m_nodeIdInfos.find(nodeId);
    if (m_nodeIdInfos.end() == iter)
    {
        LOG->write(utilities::LogType::Log_Error, FILE_INFO, "host node id not exist, id: ", nodeId);
        return -1;
    }

    if (sessionId != iter->second)
    {
        LOG->write(utilities::LogType::Log_Error, FILE_INFO, "session id not correct, id: ", nodeId, ", session id: ", sessionId,
                   ", expected session id: ", iter->second);
        return -1;
    }

    m_nodeIdInfos.erase(iter);

    return 0;
}

int HostsInfoManager::setHostNotConnected(const HostEndPointInfo& endPointInfo)
{
    std::unique_lock<std::mutex> ulock(x_hosts);
    auto iter = m_hosts.find(endPointInfo);
    if (m_hosts.end() == iter)
    {
        LOG->write(utilities::LogType::Log_Error, FILE_INFO, "host not exist");
        return -1;
    }
    iter->second.first.clear();
    iter->second.second = -1;

    return 0;
}

int HostsInfoManager::getSessionId(const NodeId& nodeId, SessionId& sessionId)
{
    std::unique_lock<std::mutex> ulock(x_hosts);

    auto iter = m_nodeIdInfos.find(nodeId);
    if (m_nodeIdInfos.end() == iter)
    {
        LOG->write(utilities::LogType::Log_Error, FILE_INFO, "host not exist");
        return -1;
    }

    sessionId = iter->second;

    return 0;
}

bool HostsInfoManager::isHostIdExist(const NodeId& nodeId)
{
    std::unique_lock<std::mutex> ulock(x_hosts);
    auto iter = m_nodeIdInfos.find(nodeId);
    if (m_nodeIdInfos.end() == iter)
    {
        return false;
    }
    return true;
}

std::vector<csm::NodeId> HostsInfoManager::onlineNodeIds()
{
    std::unique_lock<std::mutex> ulock(x_hosts);

    std::vector<NodeId> nodeIds;
    for (auto iter = m_nodeIdInfos.begin(); iter != m_nodeIdInfos.end(); ++iter)
    {
        nodeIds.emplace_back(iter->first);
    }

    return nodeIds;
}

std::vector<std::pair<csm::NodeId, SessionId>> HostsInfoManager::getAllHosts()
{
    std::vector<std::pair<NodeId, SessionId>> onlineHosts;

    std::unique_lock<std::mutex> ulock(x_hosts);
    for (const auto& [nodeId, sessionId] : m_nodeIdInfos)
    {
        onlineHosts.emplace_back(nodeId, sessionId);
    }

    return onlineHosts;
}