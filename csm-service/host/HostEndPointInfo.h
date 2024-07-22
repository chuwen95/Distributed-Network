//
// Created by root on 9/25/23.
//

#ifndef TCPNETWORK_HOSTENDPOINTINFO_H
#define TCPNETWORK_HOSTENDPOINTINFO_H

#include "csm-common/Common.h"
#include "csm-utilities/StringTool.h"

namespace csm
{

    namespace service
    {

        class HostEndPointInfo
        {
        public:
            HostEndPointInfo() = default;

            HostEndPointInfo(const std::string &ip, unsigned short port) : m_ip(ip), m_port(port)
            {}

            HostEndPointInfo(const std::string &host)
            {
                std::vector<std::string> values = csm::utilities::split(host, ':');
                if (2 == values.size())
                {
                    m_ip = values[0];
                    m_port = csm::components::convertFromString<unsigned short>(values[1]);
                }
            }

            ~HostEndPointInfo() = default;

            bool operator<(const HostEndPointInfo &rhs) const
            {
                return m_ip + std::to_string(m_port) < rhs.m_ip + std::to_string(rhs.m_port);
            }

            bool operator==(const HostEndPointInfo &rhs) const
            {
                return m_ip + std::to_string(m_port) == rhs.m_ip + std::to_string(rhs.m_port);
            }

            struct hashFunction
            {
                std::size_t operator()(const HostEndPointInfo &info) const
                {
                    return std::hash<std::string>{}(info.ip()) ^ std::hash<unsigned short>{}(info.port());
                }
            };

            std::string ip() const
            { return m_ip; }

            void setIp(const std::string &ip)
            { m_ip = ip; }

            unsigned short port() const
            { return m_port; }

            void setPort(const unsigned short port)
            { m_port = port; }

            std::string host() const
            { return m_ip + ":" + std::to_string(m_port); }

        private:
            std::string m_ip;
            unsigned short m_port;
        };

        const auto hostEndPointInfoHashFunc = [](const HostEndPointInfo &info) {
            return std::hash<std::string>{}(info.ip()) ^ std::hash<unsigned short>{}(info.port());
        };

    }

}

#endif //TCPNETWORK_HOSTENDPOINTINFO_H
