//
// Created by ChuWen on 9/16/23.
//

#ifndef TCPNETWORK_PAYLOADCLIENTINFOREPLY_H
#define TCPNETWORK_PAYLOADCLIENTINFOREPLY_H

#include "protocol/pb/clientinfo.pb.h"
#include "PayloadBase.h"

namespace csm
{

    namespace service
    {

        class PayloadClientInfoReply : public PayloadTypeBase<protocol::ClientInfoReply>
        {
        public:
            using Ptr = std::shared_ptr<PayloadClientInfoReply>;

            PayloadClientInfoReply() = default;
            PayloadClientInfoReply(std::shared_ptr<std::vector<char>> data);

        public:
            void setSeq(const std::uint32_t seq);
            std::uint32_t seq() const;

            void setPeerHost(const std::string &host);
            std::string peerHost() const;

            void setHandshakeUuid(const std::string &uuid);
            std::string handshakeUuid() const;

            void setNodeId(const std::string &id);
            std::string nodeId() const;

            void setResult(const std::int32_t result);
            std::int32_t result() const;
        };

    } // service

}

#endif //TCPNETWORK_PAYLOADCLIENTINFOREPLY_H
