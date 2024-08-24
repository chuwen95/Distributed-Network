//
// Created by ChuWen on 9/16/23.
//

#ifndef TCPNETWORK_PAYLOADCLIENTINFO_H
#define TCPNETWORK_PAYLOADCLIENTINFO_H

#include "PayloadBase.h"
#include "protocol/pb/clientinfo.pb.h"

namespace csm
{

    namespace service
    {

        class PayloadClientInfo : public PayloadBase
        {
        public:
            using Ptr = std::shared_ptr<PayloadClientInfo>;

            PayloadClientInfo() = default;
            PayloadClientInfo(std::shared_ptr<std::vector<char>> data);
            ~PayloadClientInfo() = default;

        public:
            std::size_t packetLength() const override;

            int setSeq(const std::uint32_t seq);

            std::uint32_t seq();

            int setLocalHost(const std::string &host);

            std::string localHost();

            int setPeerHost(const std::string &host);

            std::string peerHost();

            int setHandshakeUuid(const std::string &uuid);

            std::string handshakeUuid();

            int setNodeId(const std::string &id);

            std::string nodeId();

            int encode(char *buffer, const std::size_t length) const;

            int decode(const char *buffer, const std::size_t length);

        private:
            protocol::ClientInfo m_protoClientInfo;
        };

    } // service

}

#endif // TCPNETWORK_PAYLOADCLIENTINFO_H
