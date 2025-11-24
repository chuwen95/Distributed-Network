//
// Created by chu on 11/3/25.
//

#ifndef COPYSTATEMACHINE_RPCFACTORY_H
#define COPYSTATEMACHINE_RPCFACTORY_H

#include <memory>

#include "csm-rpc/framework/RpcServer.h"

namespace csm
{

    namespace rpc
    {

        class RpcFactory
        {
        public:
            virtual ~RpcFactory() = default;

        public:
            virtual std::unique_ptr<RpcServer> create() = 0;
        };

    }

}

#endif //COPYSTATEMACHINE_RPCFACTORY_H