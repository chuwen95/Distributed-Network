//
// Created by ChuWen on 9/11/23.
//

#ifndef TCPNETWORK_PACKETHEADER_H
#define TCPNETWORK_PACKETHEADER_H

#include "csm-common/Common.h"

#include "csm-framework/protocol/Protocol.h"

namespace csm
{

    namespace service
    {

        constexpr std::uint32_t c_magic{ 0x2cc98f5d };

        enum class PacketType
        {
            PT_None,

            // handshake
            PT_ClientInfo,
            PT_ClientInfoReply,

            // heartbeat
            PT_HeartBeat,
            PT_HeartBeatReply,

            // message
            PT_ModuleMessage
        };

        class PacketHeader
        {
        public:
            using Ptr = std::shared_ptr<PacketHeader>;

            PacketHeader() = default;
            ~PacketHeader() = default;

        public:
            bool isMagicMatch();

            int setType(PacketType type);
            PacketType type() const;

            int setPayloadLength(std::uint32_t payloadLength);
            std::uint32_t payloadLength() const;

            int setModuleId(protocol::ModuleID moduleId);
            protocol::ModuleID moduleId() const;

            std::size_t headerLength();

            int encode(char *buffer, const std::size_t length);
            int decode(const char *buffer, const std::size_t length);

        private:
            std::uint32_t m_magic{ c_magic };
            std::uint16_t m_packetType;
            protocol::ModuleID m_moduleId;
            std::uint32_t m_payloadLength;
        };

    } // packetprocess

}

#endif //TCPNETWORK_PACKETHEADER_H
