//
// Created by ChuWen on 9/16/23.
//

#ifndef TCPNETWORK_PACKETCLIENTINFOREPLY_H
#define TCPNETWORK_PACKETCLIENTINFOREPLY_H

#include "protocol/pb/clientinfo.pb.h"

namespace csm
{

    namespace service
    {

        class PacketClientInfoReply
        {
        public:
            using Ptr = std::shared_ptr<PacketClientInfoReply>;

            PacketClientInfoReply() = default;
            PacketClientInfoReply(std::shared_ptr<std::vector<char>> data);
            ~PacketClientInfoReply() = default;

        public:
            std::size_t packetLength() const;

            int setSeq(const std::uint32_t seq);

            std::uint32_t seq();

            int setPeerHost(const std::string &host);

            std::string peerHost();

            int setHandshakeUuid(const std::string &uuid);

            std::string handshakeUuid();

            int setNodeId(const std::string &id);

            std::string nodeId();

            int setResult(const std::int32_t result);

            std::int32_t result();

            int encode(char *buffer, const std::size_t length) const;

            int decode(const char *buffer, const std::size_t length);

        private:
            protocol::ClientInfoReply m_protoClientInfoReply;
        };

    } // packetprocess

}

#endif //TCPNETWORK_PACKETCLIENTINFOREPLY_H
