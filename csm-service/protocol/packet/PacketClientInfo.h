//
// Created by root on 9/16/23.
//

#ifndef TCPNETWORK_PACKETCLIENTINFO_H
#define TCPNETWORK_PACKETCLIENTINFO_H

#include "protocol/pb/clientinfo.pb.h"

namespace csm
{

    namespace service
    {

        class PacketClientInfo
        {
        public:
            using Ptr = std::shared_ptr<PacketClientInfo>;

            PacketClientInfo() = default;
            PacketClientInfo(std::shared_ptr<std::vector<char>> data);
            ~PacketClientInfo() = default;

        public:
            std::size_t packetLength() const;

            int setSeq(const std::uint32_t seq);

            std::uint32_t seq();

            int setLocalHost(const std::string &host);

            std::string localHost();

            int setPeerHost(const std::string &host);

            std::string peerHost();

            int setHandshakeUuid(const std::string &uuid);

            std::string handshakeUuid();

            int setNodeId(const std::string &id);

            std::string nodeId();

            int encode(char *buffer, const std::size_t length) const;

            int decode(const char *buffer, const std::size_t length);

        private:
            protocol::ClientInfo m_protoClientInfo;
        };

    } // packetprocess

}

#endif //TCPNETWORK_PACKETCLIENTINFO_H
