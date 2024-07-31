//
// Created by ChuWen on 9/11/23.
//

#ifndef TCPNETWORK_PACKETRAWSTRING_H
#define TCPNETWORK_PACKETRAWSTRING_H

#include "protocol/rawstring.pb.h"

namespace csm
{

    namespace packetprocess
    {

        class PacketRawString
        {
        public:
            using Ptr = std::shared_ptr<PacketRawString>;

            PacketRawString() = default;

            ~PacketRawString() = default;

        public:
            std::size_t packetLength() const;

            int setContent(const std::string &content);

            std::string getContent();

            int encode(char *buffer, const std::size_t length) const;

            int decode(const char *buffer, const std::size_t length);

        private:
            protocol::RawString m_protoRawString;
        };

    } // packetprocess

}

#endif //TCPNETWORK_PACKETRAWSTRING_H
