//
// Created by chu on 7/9/25.
//

#include "DistanceVector.h"

#include <algorithm>

#include "csm-service/protocol/payload/PayloadDistanceDetect.h"
#include "csm-service/protocol/payload/PayloadDistanceDetectReply.h"
#include "csm-service/protocol/payload/PayloadDistanceVector.h"
#include "csm-service/protocol/utilities/PacketEncodeHelper.h"
#include "csm-utilities/Logger.h"
#include "csm-utilities/TimeTools.h"

using namespace csm::service;

// 发送距离探测包间隔
constexpr int c_distanceDetectInterval{100};
// 向每个节点发送距离探测包间隔
constexpr int c_nodeDistanceDetectInterval{10};

// 时间转距离量化单位，毫秒
constexpr int c_timeResolutionMs{200};

DistanceVector::DistanceVector(const NodeIds& nodeIds) : m_neighbours(nodeIds)
{
    std::ranges::for_each(nodeIds, [this](const NodeId& nodeId)
    {
        m_dvInfos[nodeId].distance = c_unreachableDistance;
    });
}

int DistanceVector::init()
{
    const auto expression = [this]()
    {
        static int send_distance_detect_time{0};

        if (25 == send_distance_detect_time) //
        {
            sendDistanceDetect();
            send_distance_detect_time = 0;
        }
        ++send_distance_detect_time;

        if (true == m_needToSyncDistaceVector)
        {
            sendDistanceVector();
            m_needToSyncDistaceVector = false;
        }
    };
    m_thread = std::make_unique<utilities::Thread>(expression, c_distanceDetectInterval, "dis_vec_algo");

    return 0;
}

int DistanceVector::start()
{
    m_thread->start();

    return 0;
}

void DistanceVector::stop()
{
    m_thread->stop();
}

void DistanceVector::setPacketSender(std::function<int(const NodeId& nodeId, const std::vector<char>& data)> sender)
{
    m_packetSender = std::move(sender);
}

int DistanceVector::handlePacket(const NodeId& fromNodeId, PacketHeader::Ptr header, PayloadBase::Ptr payload)
{
    if (PacketType::PT_DistanceDetect == header->type())
    {
        return handleDistanceDetect(fromNodeId, payload);
    }
    else if (PacketType::PT_DistanceDetectReply == header->type())
    {
        return handleDistanceDetectReply(fromNodeId, payload);
    }
    else
    {
        LOG->write(utilities::LogType::Log_Error, FILE_INFO, "Received invalid packet type", header->type());
        return -1;
    }
}

void DistanceVector::sendDistanceDetect()
{
    // 构造距离探测包
    PayloadDistanceDetect payloadDistanceDetect;
    payloadDistanceDetect.setSeq(m_distanceDetectSeq++);
    payloadDistanceDetect.setTimestamp(utilities::TimeTools::getCurrentTimestamp());

    std::vector<char> buffer =
        PacketEncodeHelper<PacketType::PT_DistanceDetect, PayloadDistanceDetect>::encode(payloadDistanceDetect);
    for (const NodeId& nodeId : m_neighbours)
    {
        if (-1 == m_packetSender(nodeId, buffer))
        {
            LOG->write(utilities::LogType::Log_Error, FILE_INFO, "send distance detect failed", ", to nodeId: ",
                       nodeId);
            break;
        }

        LOG->write(utilities::LogType::Log_Error, FILE_INFO, "send distance detect successfully", ", to nodeId: ",
                   nodeId);

        std::this_thread::sleep_for(std::chrono::milliseconds(c_nodeDistanceDetectInterval));
    }
}

void DistanceVector::sendDistanceVector()
{
    PayloadDistanceVector payloadDistanceVector;
    for (const auto& [nodeId, nodeInfo] : m_dvInfos)
    {
        auto iter = m_neighboursDVInfo.find(nodeInfo.nextHop);
        if (m_neighboursDVInfo.end() != iter && nodeInfo.distance >= c_unreachableDistance)
        {
            payloadDistanceVector.addDistanceInfo(nodeId, c_unreachableDistance);
        }
        else
        {
            payloadDistanceVector.addDistanceInfo(nodeId, nodeInfo.distance);
        }
    }

    std::vector<char> buffer =
        PacketEncodeHelper<PacketType::PT_DistanceVector, PayloadDistanceVector>::encode(payloadDistanceVector);
    for (const auto& nodeId : m_neighbours)
    {
        if (0 != m_packetSender(nodeId, buffer))
        {
            LOG->write(utilities::LogType::Log_Error, FILE_INFO, "sync distance vector to node failed, nodeId: ",
                       nodeId);
            continue;
        }

        LOG->write(utilities::LogType::Log_Info, FILE_INFO, "sync distance vector to node successfully, nodeId: ",
                   nodeId);
    }
}

int DistanceVector::handleDistanceDetect(const NodeId& fromNodeId, const PayloadBase::Ptr& payload)
{
    PayloadDistanceDetect::Ptr payloadDistanceDetect = std::dynamic_pointer_cast<PayloadDistanceDetect>(payload);

    LOG->write(utilities::LogType::Log_Info, FILE_INFO, "received distance detect from node id: ", fromNodeId);

    // 构造距离探测包
    PayloadDistanceDetectReply payloadDistanceDetectReply;
    payloadDistanceDetectReply.setSeq(payloadDistanceDetect->seq());
    payloadDistanceDetectReply.setElapsedTime(
        utilities::TimeTools::getCurrentTimestamp() - payloadDistanceDetect->timestamp());

    std::vector<char> buffer =
        PacketEncodeHelper<PacketType::PT_DistanceDetectReply, PayloadDistanceDetectReply>::encode(
            payloadDistanceDetectReply);
    if (nullptr != m_packetSender)
    {
        if (0 != m_packetSender(fromNodeId, buffer))
        {
            LOG->write(utilities::LogType::Log_Error, FILE_INFO, "send distance detect reply failed, send to: ",
                       fromNodeId);
            return -1;
        }
        LOG->write(utilities::LogType::Log_Info, FILE_INFO, "send distance detect reply successfully, send to: ",
                   fromNodeId,
                   ", elapsed time: ", payloadDistanceDetectReply.elapsedTime());
    }

    return 0;
}

// 处理邻居发回来的距离探测回应包
int DistanceVector::handleDistanceDetectReply(const NodeId& fromNodeId, const PayloadBase::Ptr& payload)
{
    PayloadDistanceDetectReply::Ptr payloadDistanceDetectReply = std::dynamic_pointer_cast<
        PayloadDistanceDetectReply>(payload);

    if (payloadDistanceDetectReply->seq() < m_distanceDetectSeq)
    {
        LOG->write(utilities::LogType::Log_Warning, FILE_INFO,
                   "received distance detect reply but seq to old, ignore, from: ", fromNodeId);
        return 0;
    }

    LOG->write(utilities::LogType::Log_Info, FILE_INFO, "received distance detect reply, from: ", fromNodeId,
               ", elapsed time: ", payloadDistanceDetectReply->elapsedTime());

    std::unique_lock<std::mutex> ulock(x_dvInfos);

    auto iter = m_dvInfos.find(fromNodeId);
    if (m_dvInfos.end() != iter)
    {
        int distance = static_cast<int>(payloadDistanceDetectReply->elapsedTime()) / c_timeResolutionMs + 1;
        LOG->write(utilities::LogType::Log_Info, FILE_INFO, "received distance detect reply, from: ", fromNodeId,
                   ", elapsed time: ", payloadDistanceDetectReply->elapsedTime(), ", distance: ", distance);
        if (distance != iter->second.distance)
        {
            iter->second.distance = distance;
            m_needToSyncDistaceVector = true;
        }
    }
    else
    {
        LOG->write(utilities::LogType::Log_Error, FILE_INFO, "node not found in neighbour map");
        return -1;
    }

    std::ranges::for_each(m_dvInfos, [](const auto& dvInfo)
    {
        LOG->write(utilities::LogType::Log_Info, FILE_INFO, "node id: ", dvInfo.first, ", distance: ",
                   dvInfo.second.distance);
    });

    return 0;
}

int DistanceVector::handleDistanceVector(const NodeId& fromNodeId, const PayloadBase::Ptr& payload)
{
    PayloadDistanceVector::Ptr payloadDistanceVector = std::dynamic_pointer_cast<PayloadDistanceVector>(payload);

    std::unique_lock<std::mutex> ulock(x_dvInfos);

    bool needSendDistanceVector{false};
    for (const auto& distanceInfo : payloadDistanceVector->distanceInfos())
    {
        for (auto& dvInfo : m_dvInfos)
        {
            if (dvInfo.first == distanceInfo.first)
            {
                std::uint32_t distance{0};
                if (false == queryNodeDistanceWithoutLock(distanceInfo.first, distance))
                {
                    LOG->write(utilities::LogType::Log_Error, FILE_INFO, "not found distance info");
                    break;
                }

                if (distance + distanceInfo.second < distance)
                {
                    dvInfo.second.distance = distanceInfo.second;
                    dvInfo.second.nextHop = fromNodeId;
                }

                needSendDistanceVector = true;
            }
        }
    }

    auto iter = m_neighboursDVInfo.find(fromNodeId);
    if (m_neighboursDVInfo.end() != iter)
    {
        iter->second.clear();
        for (const auto& distanceInfo : payloadDistanceVector->distanceInfos())
        {
            iter->second.emplace(distanceInfo.first, distanceInfo.second);
        }
    }

    m_needToSyncDistaceVector = needSendDistanceVector;

    return 0;
}

bool DistanceVector::queryNodeDistanceWithoutLock(const NodeId& nodeId, std::uint32_t& distance)
{
    if (m_dvInfos.end() != m_dvInfos.find(nodeId))
    {
        distance = m_dvInfos[nodeId].distance;
        return true;
    }

    return false;
}
