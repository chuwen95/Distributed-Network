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

            int setId(const std::string &id);

            std::string getId();

            int setTimestamp(const std::uint32_t timestamp);

            std::uint32_t getTimestamp();

            int encode(char *buffer, const std::size_t length) const override;

            int decode(const char *buffer, const std::size_t length) override;

        private:
            protocol::HeartBeat m_protoHeartBeat;
        };

    } // packetprocess

}

#endif //TCPNETWORK_PACKETHEARTBEAT_H
