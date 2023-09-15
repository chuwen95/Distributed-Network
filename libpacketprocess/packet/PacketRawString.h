//
// Created by root on 9/11/23.
//

#ifndef TCPNETWORK_PACKETRAWSTRING_H
#define TCPNETWORK_PACKETRAWSTRING_H

#include "PacketBase.h"

#include "protocol/rawstring.pb.h"

namespace packetprocess
{

    class PacketRawString : public PacketBase
    {
    public:
        using Ptr = std::shared_ptr<PacketRawString>;

        PacketRawString() = default;
        ~PacketRawString() = default;

    public:
        std::size_t packetLength() override;

        int setContent(const std::string& content);
        std::string getContent();

        int encode(char* buffer, const std::size_t length) override;
        int decode(const char* buffer, const std::size_t length) override;

    private:
        protocol::RawString m_protoRawString;
    };

} // packetprocess

#endif //TCPNETWORK_PACKETRAWSTRING_H
