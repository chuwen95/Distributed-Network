//
// Created by ChuWen on 9/11/23.
//

#ifndef TCPNETWORK_PAYLOADHEARTBEAT_H
#define TCPNETWORK_PAYLOADHEARTBEAT_H

#include "protocol/pb/heartbeat.pb.h"
#include "PayloadBase.h"

namespace csm
{

    namespace service
    {

        class PayloadHeartBeat : public PayloadTypeBase<protocol::HeartBeat>
        {
        public:
            using Ptr = std::shared_ptr<PayloadHeartBeat>;

        public:
            void setNodeId(const std::string &nodeId);
            std::string nodeId() const;

            void setSeq(const std::uint64_t seq);
            std::uint64_t seq() const;

            void setTimestamp(const std::uint32_t timestamp);
            std::uint32_t timestamp() const;
        };

    } // packetprocess

}

#endif //TCPNETWORK_PACKETHEARTBEAT_H
