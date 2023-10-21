//
// Created by root on 9/16/23.
//

#ifndef TCPNETWORK_PACKETCLIENTINFO_H
#define TCPNETWORK_PACKETCLIENTINFO_H

#include "PacketBase.h"

#include "protocol/clientinfo.pb.h"

namespace packetprocess
{

    class PacketClientInfo : public PacketBase
    {
    public:
        using Ptr = std::shared_ptr<PacketClientInfo>;

        PacketClientInfo() = default;
        ~PacketClientInfo() = default;

    public:
        std::size_t packetLength() const override;

        int setSeq(const std::uint32_t seq);
        std::uint32_t seq();

        int setLocalHost(const std::string& host);
        std::string localHost();

        int setPeerHost(const std::string& host);
        std::string peerHost();

        int setHandshakeUuid(const std::string& uuid);
        std::string handshakeUuid();

        int setNodeId(const std::string& id);
        std::string nodeId();

        int encode(char* buffer, const std::size_t length) const override;
        int decode(const char* buffer, const std::size_t length) override;

    private:
        protocol::ClientInfo m_protoClientInfo;
    };

} // packetprocess

#endif //TCPNETWORK_PACKETCLIENTINFO_H
