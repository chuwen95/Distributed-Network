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

        class PayloadHeartBeatReply : public PayloadBase
        {
        public:
            using Ptr = std::shared_ptr<PayloadHeartBeatReply>;

            PayloadHeartBeatReply() = default;
            ~PayloadHeartBeatReply() = default;

        public:
            std::size_t packetLength() const override;

            int setId(const std::string &id);

            std::string getId();

            int setSendTimestamp(const std::uint32_t timestamp);

            std::uint32_t getSendTimestamp();

            int setRecvTimestamp(const std::uint32_t timestamp);

            std::uint32_t getRecvTimestamp();

            int encode(char *buffer, const std::size_t length) const override;

            int decode(const char *buffer, const std::size_t length) override;

        private:
            protocol::HeartBeatReply m_protoHeartBeatReply;
            std::vector<char> m_buffer;
        };

    } // service

}

#endif //TCPNETWORK_PAYLOADHEARTBEATREPLY_H
