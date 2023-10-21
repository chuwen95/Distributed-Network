//
// Created by root on 10/11/23.
//

#ifndef TCPNETWORK_TCPSERVICEINITIALIZER_H
#define TCPNETWORK_TCPSERVICEINITIALIZER_H

#include "libcommon/Common.h"

#include "libtools/NodeConfig.h"
#include "libservice/TcpService.h"
#include "libpacketprocess/PacketProcessor.h"

namespace initializer
{

    class TcpServiceInitializer
    {
    public:
        using Ptr = std::shared_ptr<TcpServiceInitializer>;

        TcpServiceInitializer(tool::NodeConfig::Ptr nodeConfig, packetprocess::PacketProcessor::Ptr packetProcessor);
        ~TcpServiceInitializer() = default;

    public:
        int init();

        int uninit();

        int start();

        int stop();

        service::TcpService::Ptr tcpService();

    private:
        service::TcpService::Ptr m_tcpService;
        packetprocess::PacketProcessor::Ptr m_packetProcessor;
    };

} // initializer

#endif //TCPNETWORK_TCPSERVICEINITIALIZER_H
