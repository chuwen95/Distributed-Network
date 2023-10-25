//
// Created by root on 9/11/23.
//

#ifndef TCPNETWORK_PACKETRAWSTRINGREPLY_H
#define TCPNETWORK_PACKETRAWSTRINGREPLY_H

#include "protocol/rawstring.pb.h"

namespace packetprocess
{

    class PacketRawStringReply
    {
    public:
        using Ptr = std::shared_ptr<PacketRawStringReply>;

        PacketRawStringReply() = default;
        ~PacketRawStringReply() = default;

    public:
        std::size_t packetLength() const;

        std::string getResult();
        int setResult(const std::string& result);

        int encode(char* buffer, const std::size_t length) const;
        int decode(const char* buffer, const std::size_t length);

    private:
        protocol::RawStringReply m_protoRawStringReply;
        std::vector<char> m_buffer;
    };

} // packetprocess

#endif //TCPNETWORK_PACKETRAWSTRINGREPLY_H
