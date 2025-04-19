//
// Created by chu on 3/17/25.
//

#include "ClusterConfigurationSerializer.h"

#include "json/json.h"
#include "csm-utilities/Logger.h"
#include "csm-utilities/JsonExtractHelper.h"

using namespace csm::tool;

std::string ClusterConfigurationSerializer::serialize(const consensus::NodeIds& clusterServerIds)
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

std::shared_ptr<csm::consensus::NodeIds> ClusterConfigurationSerializer::deserialize(const std::string& json)
{
    std::shared_ptr<consensus::NodeIds> clusterServerIds = std::make_shared<consensus::NodeIds>();

    Json::Value root;
    Json::Reader reader;
    if (false == reader.parse(json, root))
    {
        LOG->write(utilities::LogType::Log_Error, FILE_INFO, "parse cluster servers failed");
        return clusterServerIds;
    }

    Json::Value clusterServerIdObj;
    if (-1 == utilities::JsonExtractHelper<Json::Value>::extract(root, "clusterServerIds", clusterServerIdObj))
    {
        LOG->write(utilities::LogType::Log_Error, FILE_INFO, "parse cluster servers failed");
        return clusterServerIds;
    }

    for (auto iter = clusterServerIdObj.begin(); iter != clusterServerIdObj.end(); ++iter)
    {
        clusterServerIds->emplace_back(iter->asString());
    }

    return clusterServerIds;
}