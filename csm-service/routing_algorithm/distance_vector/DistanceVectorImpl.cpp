//
// Created by chu on 7/9/25.
//

#include "DistanceVectorImpl.h"

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
constexpr std::uint32_t c_timeResolutionMs{200};

DistanceVectorImpl::DistanceVectorImpl(const NodeIds& nodeIds) : m_distanceVector(nodeIds)
{
}

int DistanceVectorImpl::init()
{
    const auto expression = [this](std::stop_token st)
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

int DistanceVectorImpl::start()
{
    m_thread->start();

    return 0;
}

void DistanceVectorImpl::stop()
{
    m_thread->stop();
}

void DistanceVectorImpl::setPacketSender(std::function<int(const NodeId& nodeId, const std::vector<char>& data)> sender)
{
    m_packetSender = std::move(sender);
}

int DistanceVectorImpl::handlePacket(const NodeId& fromNodeId, PacketHeader::Ptr header, PayloadBase::Ptr payload)
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

void DistanceVectorImpl::sendDistanceDetect()
{
    // 构造距离探测包
    PayloadDistanceDetect payloadDistanceDetect;
    payloadDistanceDetect.setSeq(m_distanceDetectSeq++);
    payloadDistanceDetect.setTimestamp(utilities::TimeTools::getCurrentTimestamp());
    std::vector<char> buffer =
        PacketEncodeHelper<PacketType::PT_DistanceDetect, PayloadDistanceDetect>::encode(payloadDistanceDetect);

    NodeIds neighbours = m_distanceVector.neighbours();
    for (const NodeId& nodeId : neighbours)
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

void DistanceVectorImpl::sendDistanceVector()
{
    NodeIds neighbours = m_distanceVector.neighbours();
    for (const auto& nodeId : neighbours)
    {
        PayloadDistanceVector payloadDistanceVector;
        auto dvInfos = m_distanceVector.dvInfo(nodeId);
        std::ranges::for_each(dvInfos.begin(), dvInfos.end(), [&payloadDistanceVector](const auto& dvInfo)
        {
            payloadDistanceVector.addDistanceInfo(dvInfo.first, dvInfo.second);
        });

        std::vector<char> buffer =
            PacketEncodeHelper<PacketType::PT_DistanceVector, PayloadDistanceVector>::encode(payloadDistanceVector);
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

int DistanceVectorImpl::handleDistanceDetect(const NodeId& fromNodeId, const PayloadBase::Ptr& payload)
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
int DistanceVectorImpl::handleDistanceDetectReply(const NodeId& fromNodeId, const PayloadBase::Ptr& payload)
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

    // 计算距离
    std::uint32_t distance = payloadDistanceDetectReply->elapsedTime() / c_timeResolutionMs + 1;
    LOG->write(utilities::LogType::Log_Info, FILE_INFO, "received distance detect reply, from: ", fromNodeId,
               ", elapsed time: ", payloadDistanceDetectReply->elapsedTime(), ", distance: ", distance);

    // 更新距离
    m_needToSyncDistaceVector = m_distanceVector.updateNeighbourDistance(fromNodeId, distance);

    return 0;
}

// 邻居间才更新距离矢量
int DistanceVectorImpl::handleDistanceVector(const NodeId& fromNodeId, const PayloadBase::Ptr& payload)
{
    PayloadDistanceVector::Ptr payloadDistanceVector = std::dynamic_pointer_cast<PayloadDistanceVector>(payload);

    // 更新距离向量
    m_needToSyncDistaceVector = m_distanceVector.updateDvInfos(fromNodeId, payloadDistanceVector->distanceInfos());

    return 0;
}
