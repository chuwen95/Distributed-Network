//
// Created by root on 10/19/23.
//

#ifndef TCPNETWORK_RPC_H
#define TCPNETWORK_RPC_H

#include "libcommon/Common.h"
#include "config/RpcConfig.h"

namespace rpc
{

    class Rpc
    {
    public:
        using Ptr = std::shared_ptr<Rpc>;

        Rpc(RpcConfig::Ptr rpcConfig);
        ~Rpc() = default;

    public:
        int init();

        int uninit();

        int start();

        int stop();

    private:
        RpcConfig::Ptr m_rpcConfig;
    };

} // rpc

#endif //TCPNETWORK_RPC_H
