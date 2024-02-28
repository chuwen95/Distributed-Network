//
// Created by ChuWen on 2024/2/25.
//

#ifndef TCPNETWORK_LOGINITIALIZER_H
#define TCPNETWORK_LOGINITIALIZER_H

#include "libcomponents/Logger.h"
#include "libtools/NodeConfig.h"

namespace initializer
{

    class LogInitializer
    {
    public:
        using Ptr = std::shared_ptr<LogInitializer>;

        LogInitializer(tools::NodeConfig::Ptr nodeConfig);
        ~LogInitializer() = default;

    public:
        int init();

        int uninit();

        int start();

        int stop();

    private:
        tools::NodeConfig::Ptr m_nodeConfig;
    };

} // initializer

#endif //TCPNETWORK_LOGINITIALIZER_H
