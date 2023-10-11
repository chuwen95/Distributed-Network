//
// Created by root on 10/11/23.
//

#include "TcpServiceInitializer.h"

#include "libservice/TcpServiceFactory.h"
#include "libpacketprocess/PacketProcessor.h"

namespace initializer
{

    TcpServiceInitializer::TcpServiceInitializer(tool::NodeConfig::Ptr nodeConfig, packetprocess::PacketProcessor::Ptr packetProcessor)
        : m_packetProcessor(std::move(packetProcessor))
    {
        service::TcpServiceFactory tcpServiceFactory(nodeConfig);
        m_tcpService = tcpServiceFactory.createTcpService();
    }

    int TcpServiceInitializer::init()
    {
        // 忽略SIGPIPE信号，防止向一个已经断开的socket发送数据时操作系统触发SIGPIPE信号退出该应用
        signal(SIGPIPE, SIG_IGN);

        m_tcpService->registerPacketHandler([this](const packetprocess::PacketType packetType,
                packetprocess::PacketBase::Ptr packet,packetprocess::PacketReplyBase::Ptr replyPacket) {
            return m_packetProcessor->process(packetType, packet, replyPacket);
        });

        return m_tcpService->init();
    }

    int TcpServiceInitializer::uninit()
    {
        return m_tcpService->uninit();
    }

    int TcpServiceInitializer::start()
    {
        return m_tcpService->start();
    }

    int TcpServiceInitializer::stop()
    {
        return m_tcpService->stop();
    }

} // initializer