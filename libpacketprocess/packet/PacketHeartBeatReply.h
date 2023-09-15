//
// Created by root on 9/11/23.
//

#ifndef TCPNETWORK_PACKETHEARTBEATREPLY_H
#define TCPNETWORK_PACKETHEARTBEATREPLY_H

#include "PacketBase.h"

#include "protocol/heartbeat.pb.h"

namespace packetprocess
{

    class PacketHeartBeatReply : public PacketBase
    {
    public:
        using Ptr = std::shared_ptr<PacketHeartBeatReply>;

        PacketHeartBeatReply() = default;
        ~PacketHeartBeatReply() = default;

    public:
        std::size_t packetLength() override;

        int encode(char* buffer, const std::size_t length) override;
        int decode(const char* buffer, const std::size_t length) override;

    private:
        protocol::HeartBeatReply m_protoHeartBeatReply;
        std::vector<char> m_buffer;
    };

} // packetprocess

#endif //TCPNETWORK_PACKETHEARTBEATREPLY_H
