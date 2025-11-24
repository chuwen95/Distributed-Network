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
            P2PSession::Ptr create(int fd, std::size_t readBufferSize, std::size_t writeBufferSize);

        private:
            std::atomic_uint64_t m_sessionId{1};
        };

    } // namespace service

} // namespace csm

#endif // P2PSESSIONFACTORY_H
