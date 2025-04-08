//
// Created by chu on 3/17/25.
//

#include "ClusterConfigurationSerializer.h"

#include "json/json.h"
#include "csm-utilities/Logger.h"
#include "csm-utilities/JsonExtractHelper.h"

using namespace csm::tool;

std::string ClusterConfigurationSerializer::serialize(const std::vector<std::string>& clusterServerIds)
{
    Json::Value root;
    for(const std::string& serverId : clusterServerIds)
    {
        root["clusterServerIds"].append(serverId);
    }

    Json::FastWriter fastWriter;
    std::string jsonString = fastWriter.write(root);

    return jsonString;
}

std::vector<std::string> ClusterConfigurationSerializer::deserialize(const std::string& json)
{
    Json::Value root;
    Json::Reader reader;
    if (false == reader.parse(json, root))
    {
        LOG->write(csm::utilities::LogType::Log_Error, FILE_INFO, "parse cluster servers failed");
        return std::vector<std::string>();
    }

    Json::Value clusterServerIdObj;
    if (-1 == csm::utilities::JsonExtractHelper<Json::Value>::extract(root, "clusterServerIds", clusterServerIdObj))
    {
        LOG->write(csm::utilities::LogType::Log_Error, FILE_INFO, "parse cluster servers failed");
        return std::vector<std::string>();
    }

    std::vector<std::string> clusterServerIds;
    for (auto iter = clusterServerIdObj.begin(); iter != clusterServerIdObj.end(); ++iter)
    {
        clusterServerIds.emplace_back(iter->asString());
    }

    return clusterServerIds;
}