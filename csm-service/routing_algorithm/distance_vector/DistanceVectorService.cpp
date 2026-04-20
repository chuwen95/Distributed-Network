//
// Created by chu on 7/9/25.
//

#include "DistanceVectorService.h"

#include <algorithm>

#include "csm-service/protocol/utilities/PacketEncodeHelper.h"

#include "csm-utilities/Logger.h"
#include "csm-utilities/TimeTools.h"

using namespace csm::service;

// 发送距离探测包间隔
constexpr std::uint32_t c_distanceDetectInterval{2500};
// 向每个节点发送距离探测包间隔
constexpr std::uint32_t c_nodeDistanceDetectInterval{10};

// 时间转距离量化单位，毫秒
constexpr std::uint32_t c_timeResolutionMs{200};

DistanceVectorService::DistanceVectorService(NodeId selfNodeId, NodeIds nodeIds)
{
    m_distanceVector.store(std::make_shared<DistanceVector>(std::move(selfNodeId), std::move(nodeIds)),
                           std::memory_order_release);
}

int DistanceVectorService::init()
{
    const auto expression = [this](std::stop_token st)
    {
        static std::uint64_t timeInterval{c_timeResolutionMs};
        if (timeInterval >= c_distanceDetectInterval) //
        {
            sendDistanceDetect();
            m_elapsedTime.update();
        }
        timeInterval = m_elapsedTime.getElapsedTimeInMilliSec();

        Event event;
        if (true == m_eventQueue.try_dequeue(event))
        {
            std::visit(Overloaded{
                           [this](DistanceDetectReplyEvent event) { return handleDistanceDetectReply(std::move(event));},
                           [this](DistanceVectorEvent event) { return handleDistanceVector(std::move(event)); }
                       }, std::move(event));
        }
    };
    m_thread = std::make_unique<utilities::Thread>(expression, c_distanceDetectInterval, "dis_vec_algo");

    return 0;
}

void DistanceVectorService::start()
{
    m_thread->start();
}

void DistanceVectorService::stop()
{
    m_thread->stop();
}

void DistanceVectorService::setPacketSender(
    std::function<int(const NodeId& nodeId, const std::vector<char>& data)> sender)
{
    m_packetSender = std::move(sender);
}

int DistanceVectorService::handlePacket(NodeId fromNodeId, PacketHeader::Ptr header, PayloadBase::Ptr payload)
{
    if (PacketType::PT_DistanceDetect == header->type())
    {
        PayloadDistanceDetect::Ptr payloadDistanceDetect = std::dynamic_pointer_cast<PayloadDistanceDetect>(payload);
        return handleDistanceDetect(std::move(fromNodeId), std::move(payloadDistanceDetect));
    }
    else if (PacketType::PT_DistanceDetectReply == header->type())
    {
        PayloadDistanceDetectReply::Ptr payloadDistanceDetectReply = std::dynamic_pointer_cast<
            PayloadDistanceDetectReply>(payload);
        m_eventQueue.enqueue(DistanceDetectReplyEvent(std::move(fromNodeId), std::move(payloadDistanceDetectReply)));
    }
    else if (PacketType::PT_DistanceVector == header->type())
    {
        PayloadDistanceVector::Ptr payloadDistanceVector = std::dynamic_pointer_cast<PayloadDistanceVector>(payload);
        m_eventQueue.enqueue(DistanceVectorEvent(std::move(fromNodeId), std::move(payloadDistanceVector)));
    }
    else
    {
        LOG->write(utilities::LogType::Log_Error, FILE_INFO, "Received invalid packet type", header->type());
        return -1;
    }

    return 0;
}

std::optional<std::pair<Distance, csm::NodeId>> DistanceVectorService::queryRoute(const NodeId& targetNodeId)
{
    std::shared_ptr<const DistanceVector> distanceVector = m_distanceVector.load(std::memory_order_acquire);

    return distanceVector->distance(targetNodeId);
}

void DistanceVectorService::sendDistanceDetect()
{
    // 构造距离探测包
    PayloadDistanceDetect payloadDistanceDetect;
    payloadDistanceDetect.setSeq(m_distanceDetectSeq++);
    payloadDistanceDetect.setTimestamp(utilities::TimeTools::getCurrentTimestamp());
    std::vector<char> buffer =
        PacketEncodeHelper<PacketType::PT_DistanceDetect, PayloadDistanceDetect>::encode(payloadDistanceDetect);

    // 单写者（worker thread） + 多读者模型：
    // m_distanceVector 仅允许在事件处理线程中更新，其他线程只读。
    std::shared_ptr<const DistanceVector> distanceVector = m_distanceVector.load(std::memory_order_acquire);

    NodeIds neighbours = distanceVector->neighbours();
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

void DistanceVectorService::sendDistanceVector()
{
    // 单写者（worker thread） + 多读者模型：
    // m_distanceVector 仅允许在事件处理线程中更新，其他线程只读。
    std::shared_ptr<const DistanceVector> distanceVector = m_distanceVector.load(std::memory_order_acquire);

    NodeIds neighbours = distanceVector->neighbours();
    for (const auto& nodeId : neighbours)
    {
        PayloadDistanceVector payloadDistanceVector;
        auto dvInfos = distanceVector->dvInfo(nodeId);
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

int DistanceVectorService::handleDistanceDetect(NodeId fromNodeId, PayloadDistanceDetect::Ptr payload)
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
int DistanceVectorService::handleDistanceDetectReply(DistanceDetectReplyEvent event)
{
    if (event.payloadDistanceDetectReply->seq() < m_distanceDetectSeq - 1)
    {
        LOG->write(utilities::LogType::Log_Warning, FILE_INFO,
                   "received distance detect reply but seq to old, ignore, from: ", event.fromNodeId);
        return 0;
    }

    LOG->write(utilities::LogType::Log_Info, FILE_INFO, "received distance detect reply, from: ", event.fromNodeId,
               ", elapsed time: ", event.payloadDistanceDetectReply->elapsedTime());

    // 计算距离
    std::uint32_t distance = event.payloadDistanceDetectReply->elapsedTime() / c_timeResolutionMs + 1;
    LOG->write(utilities::LogType::Log_Info, FILE_INFO, "received distance detect reply, from: ", event.fromNodeId,
               ", elapsed time: ", event.payloadDistanceDetectReply->elapsedTime(), ", distance: ", distance);

    // 单写者（worker thread） + 多读者模型：
    // m_distanceVector 仅允许在事件处理线程中更新，其他线程只读。
    std::shared_ptr<DistanceVector> distanceVector = std::make_shared<DistanceVector>(
        *(m_distanceVector.load(std::memory_order_acquire)));

    // 更新距离
    bool result = distanceVector->updateNeighbourDistance(event.fromNodeId, distance);
    m_distanceVector.store(std::move(distanceVector), std::memory_order_release);

    if (result)
    {
        sendDistanceVector();
    }

    return result;
}

// 邻居间才更新距离矢量
int DistanceVectorService::handleDistanceVector(DistanceVectorEvent event)
{
    // 单写者（worker thread） + 多读者模型：
    // m_distanceVector 仅允许在事件处理线程中更新，其他线程只读。
    std::shared_ptr<DistanceVector> distanceVector = std::make_shared<DistanceVector>(
        *(m_distanceVector.load(std::memory_order_acquire)));

    // 更新距离向量
    bool result = distanceVector->updateDvInfos(event.fromNodeId, event.payloadDistanceVector->distanceInfos());

    m_distanceVector.store(std::move(distanceVector), std::memory_order_release);

    if (result)
    {
        sendDistanceVector();
    }

    return result;
}
