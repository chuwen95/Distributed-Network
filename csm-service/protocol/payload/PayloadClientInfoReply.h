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

        class PayloadClientInfoReply : public PayloadTypeBase<protocol::ClientInfoReply>
        {
        public:
            using Ptr = std::shared_ptr<PayloadClientInfoReply>;

        public:
            void setSeq(std::uint32_t seq);
            std::uint32_t seq() const;

            void setPeerHost(const std::string& host);
            std::string peerHost() const;

            void setHandshakeUuid(const std::string& uuid);
            std::string handshakeUuid() const;

            void setNodeId(const std::string& id);
            std::string nodeId() const;

            void setResult(const std::int32_t result);
            std::int32_t result() const;
        };

    } // namespace service

} // namespace csm

#endif // TCPNETWORK_PAYLOADCLIENTINFOREPLY_H
