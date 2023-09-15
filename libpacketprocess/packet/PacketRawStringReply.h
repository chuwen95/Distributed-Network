//
// Created by root on 9/11/23.
//

#ifndef TCPNETWORK_PACKETRAWSTRINGREPLY_H
#define TCPNETWORK_PACKETRAWSTRINGREPLY_H

#include "PacketBase.h"

#include "protocol/rawstring.pb.h"

namespace packetprocess
{

    class PacketRawStringReply : public PacketBase
    {
    public:
        using Ptr = std::shared_ptr<PacketRawStringReply>;

        PacketRawStringReply() = default;
        ~PacketRawStringReply() = default;

    public:
        std::size_t packetLength() override;

        std::string getResult();
        int setResult(const std::string& result);

        int encode(char* buffer, const std::size_t length) override;
        int decode(const char* buffer, const std::size_t length) override;

    private:
        protocol::RawStringReply m_protoRawStringReply;
        std::vector<char> m_buffer;
    };

} // packetprocess

#endif //TCPNETWORK_PACKETRAWSTRINGREPLY_H
