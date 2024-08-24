//
// Created by ChuWen on 2024/2/27.
//

#ifndef TCPNETWORK_TCPSESSIONFACTORY_H
#define TCPNETWORK_TCPSESSIONFACTORY_H

#include "TcpSession.h"

namespace csm
{

    namespace service
    {

        class TcpSessionFactory
        {
        public:
            TcpSession::Ptr createTcpSession(const int fd, const std::size_t readBufferSize, const std::size_t writeBufferSize);
        };

    } // service

}

#endif //TCPNETWORK_TCPSESSIONFACTORY_H
