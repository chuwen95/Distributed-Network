//
// Created by ChuWen on 10/11/23.
//

#include "TcpServiceInitializer.h"

#include "csm-utilities/Logger.h"
#include "csm-service/TcpServiceFactory.h"
#include "csm-packetprocess/packet/PacketRawString.h"

using namespace csm::initializer;

std::atomic_int receiveNum{0};

TcpServiceInitializer::TcpServiceInitializer(tool::NodeConfig::Ptr nodeConfig)
{
    service::TcpServiceFactory tcpServiceFactory(nodeConfig);
    m_tcpService = tcpServiceFactory.createTcpService();
}

int TcpServiceInitializer::init()
{
    // 忽略SIGPIPE信号，防止向一个已经断开的socket发送数据时操作系统触发SIGPIPE信号退出该应用
    signal(SIGPIPE, SIG_IGN);

    m_tcpService->registerModulePacketHandler(1, [&](std::shared_ptr<std::vector<char>> data) -> int {
        packetprocess::PacketRawString packetRawString;
        packetRawString.decode(data->data(), data->size());

#if 0
        LOG->write(utilities::LogType::Log_Info, FILE_INFO,
                                                                     "content: ", packetRawString.getContent());
#endif
        if ("hello world" == packetRawString.getContent())
        {
            ++receiveNum;
            if (receiveNum % 5000 == 0)
            {
                std::cout << "5000 payload received" << std::endl;
                receiveNum = 0;
            }
        }

        return 0;
    });

    return m_tcpService->init();
}

int TcpServiceInitializer::start()
{
    return m_tcpService->start();
}

int TcpServiceInitializer::stop()
{
    return m_tcpService->stop();
}

csm::service::TcpService::Ptr TcpServiceInitializer::tcpService()
{
    return m_tcpService;
}