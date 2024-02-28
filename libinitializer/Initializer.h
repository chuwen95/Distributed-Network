//
// Created by root on 10/11/23.
//

#ifndef TCPNETWORK_INITIALIZER_H
#define TCPNETWORK_INITIALIZER_H

#include "LogInitializer.h"
#include "TcpServiceInitializer.h"
#include "RpcInitializer.h"

namespace initializer
{

    class Initializer
    {
    public:
        Initializer() = default;
        ~Initializer() = default;

    public:
        int initConfig(const std::string& configPath);

        int init();

        int uninit();

        int start();

        int stop();

    public:
        tools::NodeConfig::Ptr m_nodeConfig;

        LogInitializer::Ptr m_logInitializer;
        TcpServiceInitializer::Ptr m_tcpServiceInitializer;
        RpcInitializer::Ptr m_rpcInitializer;
    };

} // initializer

#endif //TCPNETWORK_INITIALIZER_H
