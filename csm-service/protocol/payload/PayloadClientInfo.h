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

        class PayloadClientInfo : public PayloadTypeBase<protocol::ClientInfo>
        {
        public:
            using Ptr = std::shared_ptr<PayloadClientInfo>;

        public:
            void setSeq(std::uint32_t seq);
            std::uint32_t seq() const;

            void setLocalHost(const std::string& host);
            std::string localHost() const;

            void setPeerHost(const std::string& host);
            std::string peerHost() const;

            void setHandshakeUuid(const std::string& uuid);
            std::string handshakeUuid() const;

            void setNodeId(const std::string& id);
            std::string nodeId() const;
        };

    } // namespace service

} // namespace csm

#endif // TCPNETWORK_PAYLOADCLIENTINFO_H
