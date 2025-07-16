//
// Created by ChuWen on 9/11/23.
//

#ifndef TCPNETWORK_PAYLOADHEARTBEATREPLY_H
#define TCPNETWORK_PAYLOADHEARTBEATREPLY_H

#include "protocol/pb/heartbeat.pb.h"
#include "PayloadBase.h"

namespace csm
{

    namespace service
    {

        class PayloadHeartBeatReply : public PayloadTypeBase<protocol::HeartBeatReply>
        {
        public:
            using Ptr = std::shared_ptr<PayloadHeartBeatReply>;

        public:
            void setNodeId(const std::string &id);
            std::string nodeId() const;

            void setSeq(const std::uint64_t seq);
            std::uint64_t seq() const;

            void setTransferTime(const std::uint32_t timestamp);
            std::uint32_t transferTime() const;
        };

    } // service

}

#endif //TCPNETWORK_PAYLOADHEARTBEATREPLY_H
