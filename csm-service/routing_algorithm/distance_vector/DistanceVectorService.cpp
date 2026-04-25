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
// 距离探测未回应次数阈值，大于该阈值判定邻居不可达
constexpr std::uint32_t c_distanceDetectLostThreshold{3};

DistanceVectorService::DistanceVectorService(NodeId selfNodeId, NodeIds nodeIds)
{
    m_distanceVector.store(std::make_shared<DistanceVector>(std::move(selfNodeId), std::move(nodeIds)),
                           std::memory_order_release);

    initDistanceDetectSeqInfo();
    initEventHandler();
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
        return handleDistanceDetect(DistanceDetectEvent(std::move(fromNodeId), std::move(payloadDistanceDetect)));
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
    // 单写者（worker thread） + 多读者模型：
    // m_distanceVector 仅允许在事件处理线程中更新，其他线程只读。
    std::shared_ptr<const DistanceVector> distanceVector = m_distanceVector.load(std::memory_order_acquire);

    NodeIds neighbours = distanceVector->neighbours();
    for (const NodeId& nodeId : neighbours)
    {
        auto neighbourIter = m_neighbourDistanceDetectSeqInfo.find(nodeId);
        assert(m_neighbourDistanceDetectSeqInfo.end() != neighbourIter);

        std::uint64_t lostCount = neighbourIter->second.sendSeq - neighbourIter->second.receivedSeq;
        if (lostCount > c_distanceDetectLostThreshold)
        {
            LOG->write(utilities::LogType::Log_Warning, FILE_INFO,
                       "If no distance detection response packets are received after three consecutive attempts, "
                       "it is assumed that the neighbor is currently unreachable.");

            std::shared_ptr<DistanceVector> mutableDistanceVector = std::make_shared<DistanceVector>(
                *m_distanceVector.load(std::memory_order_acquire));
            mutableDistanceVector->updateNeighbourDistance(nodeId, c_unreachableDistance);
            m_distanceVector.store(std::move(mutableDistanceVector), std::memory_order_release);

            continue;
        }

        // 构造距离探测包
        PayloadDistanceDetect payloadDistanceDetect;
        payloadDistanceDetect.setSeq(neighbourIter->second.sendSeq++);
        payloadDistanceDetect.setTimestamp(utilities::TimeTools::getCurrentTimestamp());
        std::vector<char> buffer =
            PacketEncodeHelper<PacketType::PT_DistanceDetect, PayloadDistanceDetect>::encode(payloadDistanceDetect);

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

int DistanceVectorService::handleDistanceDetect(DistanceDetectEvent event)
{
    PayloadDistanceDetect::Ptr payloadDistanceDetect = std::dynamic_pointer_cast<PayloadDistanceDetect>(
        event.payloadDistanceDetect);

    LOG->write(utilities::LogType::Log_Info, FILE_INFO, "received distance detect from node id: ", event.fromNodeId);

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
        if (0 != m_packetSender(event.fromNodeId, buffer))
        {
            LOG->write(utilities::LogType::Log_Error, FILE_INFO, "send distance detect reply failed, send to: ",
                       event.fromNodeId);
            return -1;
        }
        LOG->write(utilities::LogType::Log_Info, FILE_INFO, "send distance detect reply successfully, send to: ",
                   event.fromNodeId,
                   ", elapsed time: ", payloadDistanceDetectReply.elapsedTime());
    }

    return 0;
}

// 处理邻居发回来的距离探测回应包
int DistanceVectorService::handleDistanceDetectReply(DistanceDetectReplyEvent event)
{
    auto neighbourIter = m_neighbourDistanceDetectSeqInfo.find(event.fromNodeId);
    if (m_neighbourDistanceDetectSeqInfo.end() == neighbourIter)
    {
        LOG->write(utilities::LogType::Log_Warning, FILE_INFO,
            "The source node of the distance vector probe response packet is not in this node's neighbor list.");
        return -1;
    }

    // 判断距离向量回应包的seq是否太旧
    if (event.payloadDistanceDetectReply->seq() <= neighbourIter->second.receivedSeq)
    {
        LOG->write(utilities::LogType::Log_Warning, FILE_INFO,
                   "received distance detect reply but seq to old, ignore, from: ", event.fromNodeId);
        return 0;
    }

    // 记录最新收到的距离向量回应包的seq
    neighbourIter->second.receivedSeq = event.payloadDistanceDetectReply->seq();

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

void DistanceVectorService::initDistanceDetectSeqInfo()
{
    std::ranges::for_each(m_distanceVector.load(std::memory_order_relaxed)->neighbours(), [this](const NodeId& nodeId)
    {
        m_neighbourDistanceDetectSeqInfo[nodeId] = DistanceDetectSendReplyInfo();
    });
}

void DistanceVectorService::initEventHandler()
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
                           [this](DistanceDetectEvent event) { return handleDistanceDetect(std::move(event)); },
                           [this](DistanceDetectReplyEvent event)
                           {
                               return handleDistanceDetectReply(std::move(event));
                           },
                           [this](DistanceVectorEvent event) { return handleDistanceVector(std::move(event)); }
                       }, std::move(event));
        }
    };
    m_thread = std::make_unique<utilities::Thread>(expression, c_distanceDetectInterval, "dis_vec_algo");
}
