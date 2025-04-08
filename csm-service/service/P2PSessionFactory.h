//
// Created by ChuWen on 2024/2/27.
//

#ifndef P2PSESSIONFACTORY_H
#define P2PSESSIONFACTORY_H

#include "P2PSession.h"

namespace csm
{

    namespace service
    {

        class P2PSessionFactory
        {
        public:
            P2PSession::Ptr create(const int fd, const std::size_t readBufferSize, const std::size_t writeBufferSize);
        };

    } // service

}

#endif //P2PSESSIONFACTORY_H
