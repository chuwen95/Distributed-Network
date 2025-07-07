//
// Created by ChuWen on 9/11/23.
//

#ifndef TCPNETWORK_PAYLOADHEARTBEAT_H
#define TCPNETWORK_PAYLOADHEARTBEAT_H

#include "PayloadBase.h"
#include "protocol/pb/heartbeat.pb.h"

namespace csm
{

    namespace service
    {

        class PayloadHeartBeat : public PayloadBase
        {
        public:
            using Ptr = std::shared_ptr<PayloadHeartBeat>;

            PayloadHeartBeat() = default;
            ~PayloadHeartBeat() = default;

        public:
            std::size_t packetLength() const override;

            int setNodeId(const std::string &nodeId);
            std::string nodeId() const;

            int setSeq(const std::uint64_t seq);
            std::uint64_t seq() const;

            int setTimestamp(const std::uint32_t timestamp);
            std::uint32_t timestamp() const;

            int encode(char *buffer, const std::size_t length) const override;
            int decode(const char *buffer, const std::size_t length) override;

        private:
            service::HeartBeat m_protoHeartBeat;
        };

    } // packetprocess

}

#endif //TCPNETWORK_PACKETHEARTBEAT_H
