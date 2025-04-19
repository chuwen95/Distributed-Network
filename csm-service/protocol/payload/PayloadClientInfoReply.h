//
// Created by ChuWen on 9/16/23.
//

#ifndef TCPNETWORK_PAYLOADCLIENTINFOREPLY_H
#define TCPNETWORK_PAYLOADCLIENTINFOREPLY_H

#include "PayloadBase.h"
#include "protocol/pb/clientinfo.pb.h"

namespace csm
{

    namespace service
    {

        class PayloadClientInfoReply : public PayloadBase
        {
        public:
            using Ptr = std::shared_ptr<PayloadClientInfoReply>;

            PayloadClientInfoReply() = default;
            PayloadClientInfoReply(std::shared_ptr<std::vector<char>> data);
            ~PayloadClientInfoReply() = default;

        public:
            std::size_t packetLength() const override;

            int setSeq(const std::uint32_t seq);
            std::uint32_t seq();

            int setPeerHost(const std::string &host);
            std::string peerHost();

            int setHandshakeUuid(const std::string &uuid);
            std::string handshakeUuid();

            int setNodeId(const std::string &id);
            std::string nodeId();

            int setResult(const std::int32_t result);
            std::int32_t result();

            int encode(char *buffer, const std::size_t length) const override;
            int decode(const char *buffer, const std::size_t length) override;

        private:
            service::ClientInfoReply m_protoClientInfoReply;
        };

    } // service

}

#endif //TCPNETWORK_PAYLOADCLIENTINFOREPLY_H
