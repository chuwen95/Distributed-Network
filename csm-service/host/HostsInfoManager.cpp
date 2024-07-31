//
// Created by ChuWen on 9/22/23.
//

#include "HostsInfoManager.h"
#include "csm-utilities/Logger.h"
#include "csm-utilities/StringTool.h"
#include "csm-utilities/Timestamp.h"

#include <json/json.h>

using namespace csm::service;

int HostsInfoManager::init(const std::string &configFile)
{
    std::ifstream nodeFile(configFile, std::ios::in);
    if (false == nodeFile.is_open())
    {
        LOG->write(utilities::LogType::Log_Error, FILE_INFO, "open nodes config file failed: ", configFile);
        return -1;
    }
    nodeFile.seekg(0, std::ios::end);
    auto length = nodeFile.tellg();
    nodeFile.seekg(0, std::ios::beg);

    std::vector<char> content;
    content.resize(length);
    nodeFile.read(content.data(), length);
    nodeFile.close();

    std::string jsonContent(content.data(), length);
    LOG->write(utilities::LogType::Log_Debug, FILE_INFO, "nodes config file content: ", jsonContent);

    Json::Value root;
    Json::Reader reader;
    bool ret = reader.parse(std::string(content.data(), content.size()), root);
    if (false == ret)
    {
        LOG->write(utilities::LogType::Log_Error, FILE_INFO, "parse nodes config file failed: ", configFile);
        return -1;
    }
    LOG->write(utilities::LogType::Log_Info, FILE_INFO, "parse nodes config file successfully: ", configFile);

    Json::Value &nodesValue = root["nodes"];
    if (false == nodesValue.isArray())
    {
        LOG->write(utilities::LogType::Log_Error, FILE_INFO, "parse hosts config file failed: ", configFile);
        return -1;
    }

    for (auto iter = nodesValue.begin(); iter != nodesValue.end(); ++iter)
    {
        std::string host = iter->asString();
        LOG->write(utilities::LogType::Log_Info, FILE_INFO, "hosts info: ", host);
        std::vector<std::string> ipPort = csm::utilities::split(host, ':');
        if (2 != ipPort.size())
        {
            LOG->write(utilities::LogType::Log_Error, FILE_INFO, "parse hosts ip and port failed: ", host);
            return -1;
        }

        std::string ip = ipPort[0];
        unsigned short port = csm::utilities::convertFromString<unsigned short>(ipPort[1]);
        m_hosts.emplace(HostEndPointInfo(ip, port), std::make_pair("", -1));
    }

    return 0;
}

int HostsInfoManager::uninit()
{
    return 0;
}

auto HostsInfoManager::getHosts() -> const Hosts &
{
    std::unique_lock<std::mutex> ulock(x_hosts);
    return m_hosts;
}

int HostsInfoManager::setHostId(const service::HostEndPointInfo &endPointInfo, const std::string &id)
{
    std::unique_lock<std::mutex> ulock(x_hosts);
    auto iter = m_hosts.find(endPointInfo);
    if (m_hosts.end() == iter)
    {
        return -1;
    }
    iter->second.first = id;
    return 0;
}

int HostsInfoManager::addHostIdInfo(const std::string &id, const int fd, const std::string &uuid)
{
    std::unique_lock<std::mutex> ulock(x_hosts);
    auto iter = m_nodeIdInfos.find(id);
    if (m_nodeIdInfos.end() != iter)
    {
        LOG->write(utilities::LogType::Log_Error, FILE_INFO, "host already exist");
        return -1;
    }
    m_nodeIdInfos.emplace(id, std::make_pair(fd, uuid));

    return 0;
}

int HostsInfoManager::setHostIdInfo(const std::string &id, const int fd, const std::string &uuid)
{
    std::unique_lock<std::mutex> ulock(x_hosts);
    auto iter = m_nodeIdInfos.find(id);
    if (m_nodeIdInfos.end() == iter)
    {
        LOG->write(utilities::LogType::Log_Error, FILE_INFO, "host already exist");
        return -1;
    }
    iter->second.first = fd;
    iter->second.second = uuid;

    return 0;
}

int HostsInfoManager::removeHostIdInfo(const std::string &id, const std::string &uuid)
{
    std::unique_lock<std::mutex> ulock(x_hosts);
    auto iter = m_nodeIdInfos.find(id);
    if (m_nodeIdInfos.end() == iter)
    {
        LOG->write(utilities::LogType::Log_Error, FILE_INFO, "host id not exist, id: ", id);
        return -1;
    }
    if (iter->second.second != uuid)
    {
        LOG->write(utilities::LogType::Log_Error, FILE_INFO, "uuid not match, id: ", id);
        return -1;
    }
    m_nodeIdInfos.erase(iter);

    return 0;
}

int HostsInfoManager::setHostNotConnected(const service::HostEndPointInfo &endPointInfo)
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

int HostsInfoManager::getHostFdById(const std::string id)
{
    std::unique_lock<std::mutex> ulock(x_hosts);
    auto iter = m_nodeIdInfos.find(id);
    if (m_nodeIdInfos.end() == iter)
    {
        LOG->write(utilities::LogType::Log_Error, FILE_INFO, "host not exist");
        return -1;
    }
    return iter->second.first;
}

bool HostsInfoManager::isHostIdExist(const std::string id, int &fd, std::string &uuid)
{
    std::unique_lock<std::mutex> ulock(x_hosts);
    auto iter = m_nodeIdInfos.find(id);
    if (m_nodeIdInfos.end() == iter)
    {
        return false;
    }
    fd = iter->second.first;
    uuid = iter->second.second;
    return true;
}

bool HostsInfoManager::isHostIdExist(const std::string id)
{
    std::unique_lock<std::mutex> ulock(x_hosts);
    auto iter = m_nodeIdInfos.find(id);
    if (m_nodeIdInfos.end() == iter)
    {
        return false;
    }
    return true;
}

std::uint32_t HostsInfoManager::onlineClientSize()
{
    std::unique_lock<std::mutex> ulock(x_hosts);
    return m_nodeIdInfos.size();
}

std::vector<std::pair<std::string, int>> HostsInfoManager::getAllOnlineClients()
{
    std::vector<std::pair<std::string, int>> onlineClients;

    std::unique_lock<std::mutex> ulock(x_hosts);
    for (auto iter = m_nodeIdInfos.begin(); iter != m_nodeIdInfos.end(); ++iter)
    {
        onlineClients.emplace_back(iter->first, iter->second.first);
    }

    return std::move(onlineClients);
}

bool HostsInfoManager::waitAtLeastOneNodeConnected(const int timeout)
{
    utilities::Timestamp timestamp;
    timestamp.update();
    while (onlineClientSize() < 1 && timestamp.getElapsedTimeInMilliSec() < timeout)
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(500));
    }

    if (onlineClientSize() < 1)
    {
        return false;
    } else
    {
        return true;
    }
}