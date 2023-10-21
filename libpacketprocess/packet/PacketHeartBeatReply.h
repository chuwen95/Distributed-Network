//
// Created by root on 9/11/23.
//

#ifndef TCPNETWORK_PACKETHEARTBEATREPLY_H
#define TCPNETWORK_PACKETHEARTBEATREPLY_H

#include "PacketReplyBase.h"

#include "protocol/heartbeat.pb.h"

namespace packetprocess
{

    class PacketHeartBeatReply : public PacketReplyBase
    {
    public:
        using Ptr = std::shared_ptr<PacketHeartBeatReply>;

        PacketHeartBeatReply() = default;
        ~PacketHeartBeatReply() = default;

    public:
        std::size_t packetLength() const override;

        int setId(const std::string& id);
        std::string getId();

        int setSendTimestamp(const std::uint32_t timestamp);
        std::uint32_t getSendTimestamp();

        int setRecvTimestamp(const std::uint32_t timestamp);
        std::uint32_t getRecvTimestamp();

        int encode(char* buffer, const std::size_t length) const override;
        int decode(const char* buffer, const std::size_t length) override;

    private:
        protocol::HeartBeatReply m_protoHeartBeatReply;
        std::vector<char> m_buffer;
    };

} // packetprocess

#endif //TCPNETWORK_PACKETHEARTBEATREPLY_H
