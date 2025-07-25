//
// Created by chu on 7/17/25.
//

#ifndef PACKETENCODEHELPER_H
#define PACKETENCODEHELPER_H

#include "csm-framework/protocol/Protocol.h"
#include "csm-service/protocol/header/PacketHeader.h"

namespace csm
{

    namespace service
    {

        template <PacketType packetType, typename T> class PacketEncodeHelper
        {
        public:
            static std::vector<char> encode(const T& payload)
            {
                PacketHeader packetHeader;
                packetHeader.setType(packetType);
                packetHeader.setPayloadLength(payload.packetLength());

                std::vector<char> buffer;
                buffer.resize(packetHeader.headerLength() + payload.packetLength());

                packetHeader.encode(buffer.data(), packetHeader.headerLength());
                payload.encode(buffer.data() + packetHeader.headerLength(), payload.packetLength());

                return buffer;
            }
        };

        template <> class PacketEncodeHelper<PacketType::PT_HeartBeat, std::nullopt_t>
        {
        public:
            static std::vector<char> encode()
            {
                PacketHeader packetHeader;
                packetHeader.setType(PacketType::PT_HeartBeat);
                packetHeader.setPayloadLength(0);

                std::vector<char> buffer;
                buffer.resize(packetHeader.headerLength());

                packetHeader.encode(buffer.data(), packetHeader.headerLength());

                return buffer;
            }
        };

        template <> class PacketEncodeHelper<PacketType::PT_HeartBeatReply, std::nullopt_t>
        {
        public:
            static std::vector<char> encode()
            {
                PacketHeader packetHeader;
                packetHeader.setType(PacketType::PT_HeartBeatReply);
                packetHeader.setPayloadLength(0);

                std::vector<char> buffer;
                buffer.resize(packetHeader.headerLength());

                packetHeader.encode(buffer.data(), packetHeader.headerLength());

                return buffer;
            }
        };

        template <> class PacketEncodeHelper<PacketType::PT_ModuleMessage, std::vector<char>>
        {
        public:
            static std::vector<char> encode(const csm::protocol::ModuleID moduleId, const std::vector<char>& payload)
            {
                PacketHeader packetHeader;
                packetHeader.setType(PacketType::PT_ModuleMessage);
                packetHeader.setModuleId(moduleId);
                packetHeader.setPayloadLength(payload.size());

                // 编码包为待发送数据
                std::vector<char> buffer;
                buffer.resize(packetHeader.headerLength() + payload.size());

                packetHeader.encode(buffer.data(), packetHeader.headerLength());
                memcpy(buffer.data() + packetHeader.headerLength(), payload.data(), payload.size());

                return buffer;
            }
        };

    } // namespace service

} // namespace csm

#endif // PACKETENCODEHELPER_H
