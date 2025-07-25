//
// Created by ChuWen on 9/11/23.
//

#ifndef TCPNETWORK_PAYLOADHEARTBEATREPLY_H
#define TCPNETWORK_PAYLOADHEARTBEATREPLY_H

#include "PayloadBase.h"
#include "protocol/pb/heartbeat.pb.h"

namespace csm
{

    namespace service
    {

        class PayloadHeartBeatReply : public PayloadTypeBase<protocol::HeartBeatReply>
        {
        public:
            using Ptr = std::shared_ptr<PayloadHeartBeatReply>;

        public:
            void setNodeId(const std::string& id);
            std::string nodeId() const;

            void setSeq(std::uint64_t seq);
            std::uint64_t seq() const;

            void setTransferTime(std::uint32_t timestamp);
            std::uint32_t transferTime() const;
        };

    } // namespace service

} // namespace csm

#endif // TCPNETWORK_PAYLOADHEARTBEATREPLY_H
