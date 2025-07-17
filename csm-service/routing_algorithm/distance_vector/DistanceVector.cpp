//
// Created by chu on 7/9/25.
//

#include "DistanceVector.h"

#include "csm-service/protocol/payload/PayloadDistanceDetect.h"
#include "csm-service/protocol/payload/PayloadDistanceDetectReply.h"
#include "csm-service/protocol/utilities/PacketEncodeHelper.h"
#include "csm-utilities/TimeTools.h"
#include "csm-utilities/Logger.h"

using namespace csm::service;

constexpr int c_distanceDetectInterval{5000};
constexpr int c_nodeDistanceDetectInterval{100};

DistanceVector::DistanceVector(utilities::Thread::Ptr thread) : m_thread(std::move(thread))
{
}

int DistanceVector::init()
{
    const auto expression = [this]()
    {
        // 构造距离探测包
        PayloadDistanceDetect payloadDistanceDetect;
        payloadDistanceDetect.setSeq(m_distanceDetectSeq++);
        payloadDistanceDetect.setTimestamp(utilities::TimeTools::getCurrentTimestamp());

        std::vector<char> buffer =
            PacketEncodeHelper<PacketType::PT_DistanceDetect, PayloadDistanceDetect>::encode(payloadDistanceDetect);
        std::unordered_map<NodeId, NodeInfo::Ptr> dvInfo;
        {
            std::unique_lock<std::mutex> ulock(x_dvInfo);
            dvInfo = m_dvInfo;
        }

        for (const auto& [nodeId, nodeInfo] : dvInfo)
        {
            if (-1 == m_packetSender(nodeInfo->nodeId, buffer))
            {
                LOG->write(utilities::LogType::Log_Error, FILE_INFO,
                           "send distance detect failed", ", to nodeId: ", nodeId);
                return -1;
            }

            LOG->write(utilities::LogType::Log_Error, FILE_INFO,
                       "send distance detect successfully", ", to nodeId: ", nodeId);

            std::this_thread::sleep_for(std::chrono::milliseconds(c_nodeDistanceDetectInterval));
        }
    };
    m_thread->setFunc(expression);
    m_thread->setInterval(c_distanceDetectInterval);
    m_thread->setName("dis_vector");

    return 0;
}

int DistanceVector::start()
{
    return m_thread->start();
}

void DistanceVector::stop()
{
    m_thread->stop();
}

void DistanceVector::addNeighbourNode(const NodeId& nodeId, int nodeFd)
{
    std::unique_lock<std::mutex> ulock(x_dvInfo);
    m_dvInfo[nodeId] = std::make_shared<NodeInfo>(nodeId, nodeId);
}

void DistanceVector::setPacketSender(std::function<int(const NodeId& nodeId, const std::vector<char>& data)> sender)
{
    m_packetSender = std::move(sender);
}

int DistanceVector::handlePacket(const NodeId& fromNodeId, PacketHeader::Ptr header, PayloadBase::Ptr payload)
{
    if (PacketType::PT_DistanceDetect == header->type())
    {
        return handleDistanceDetect(fromNodeId, header, payload);
    }
    else if (PacketType::PT_DistanceDetectReply == header->type())
    {
        return handleDistanceDetectReply(fromNodeId, header, payload);
    }
    else
    {
        LOG->write(utilities::LogType::Log_Error, FILE_INFO, "Received invalid packet type", header->type());
        return -1;
    }
}

int DistanceVector::handleDistanceDetect(const NodeId& fromNodeId, const PacketHeader::Ptr& header, const PayloadBase::Ptr& payload)
{
    PayloadDistanceDetect::Ptr payloadDistanceDetect = std::dynamic_pointer_cast<PayloadDistanceDetect>(payload);

    // 构造距离探测包
    PayloadDistanceDetectReply payloadDistanceDetectReply;
    payloadDistanceDetectReply.setSeq(payloadDistanceDetect->seq());
    payloadDistanceDetectReply.setElapsedTime(utilities::TimeTools::getCurrentTimestamp() - payloadDistanceDetect->timestamp());

    std::vector<char> buffer =
        PacketEncodeHelper<PacketType::PT_DistanceDetectReply, PayloadDistanceDetectReply>::encode(payloadDistanceDetectReply);
    if (nullptr != m_packetSender)
    {
        if (0 != m_packetSender(fromNodeId, buffer))
        {
            LOG->write(utilities::LogType::Log_Error, FILE_INFO, "send distance detect reply failed, send to: ", fromNodeId);
            return -1;
        }
    }

    return 0;
}

int DistanceVector::handleDistanceDetectReply(const NodeId& fromNodeId, const PacketHeader::Ptr& header, const PayloadBase::Ptr& payload)
{
    return 0;
}