//
// Created by ChuWen on 9/16/23.
//

#ifndef TCPNETWORK_PAYLOADCLIENTINFO_H
#define TCPNETWORK_PAYLOADCLIENTINFO_H

#include "protocol/pb/clientinfo.pb.h"
#include "PayloadBase.h"

namespace csm
{

    namespace service
    {

        class PayloadClientInfo : public PayloadTypeBase<protocol::ClientInfo>
        {
        public:
            using Ptr = std::shared_ptr<PayloadClientInfo>;

            PayloadClientInfo() = default;
            PayloadClientInfo(std::shared_ptr<std::vector<char>> data);

        public:
            void setSeq(const std::uint32_t seq);
            std::uint32_t seq() const;

            void setLocalHost(const std::string &host);
            std::string localHost() const;

            void setPeerHost(const std::string &host);
            std::string peerHost() const;

            void setHandshakeUuid(const std::string &uuid);
            std::string handshakeUuid() const;

            void setNodeId(const std::string &id);
            std::string nodeId() const;
        };

    } // service

}

#endif // TCPNETWORK_PAYLOADCLIENTINFO_H
