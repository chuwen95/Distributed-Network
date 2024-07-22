//
// Created by root on 9/11/23.
//

#ifndef TCPNETWORK_PACKETHEARTBEAT_H
#define TCPNETWORK_PACKETHEARTBEAT_H

#include "protocol/pb/heartbeat.pb.h"

namespace csm
{

    namespace service
    {

        class PacketHeartBeat
        {
        public:
            using Ptr = std::shared_ptr<PacketHeartBeat>;

            PacketHeartBeat() = default;
            ~PacketHeartBeat() = default;

        public:
            std::size_t packetLength() const;

            int setId(const std::string &id);

            std::string getId();

            int setTimestamp(const std::uint32_t timestamp);

            std::uint32_t getTimestamp();

            int encode(char *buffer, const std::size_t length) const;

            int decode(const char *buffer, const std::size_t length);

        private:
            protocol::HeartBeat m_protoHeartBeat;
        };

    } // packetprocess

}

#endif //TCPNETWORK_PACKETHEARTBEAT_H
