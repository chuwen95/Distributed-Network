//
// Created by Chu on 2024/9/5.
//

#ifndef RPCSERVER_H
#define RPCSERVER_H

#include "csm-common/Common.h"

namespace csm
{

    namespace rpc
    {

        class RpcServer
        {
        public:
            virtual ~RpcServer() = default;

        public:
            virtual int init() = 0;

            virtual int start() = 0;

            virtual int stop() = 0;
        };

    }

}

#endif //RPCSERVER_H
