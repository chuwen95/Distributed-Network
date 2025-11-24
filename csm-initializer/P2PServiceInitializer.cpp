//
// Created by ChuWen on 10/11/23.
//

#include "P2PServiceInitializer.h"

#include "csm-utilities/Logger.h"
#include "csm-service/factory/P2PServiceFactory.h"
#include "csm-packetprocess/packet/PacketRawString.h"
#include "csm-framework/protocol/Protocol.h"

using namespace csm::initializer;

namespace
{
    std::atomic_int receiveNum{0};
}

P2PServiceInitializer::P2PServiceInitializer(tool::NodeConfig* nodeConfig)
{
    service::P2PServiceFactory nodeP2PServiceFactory(nodeConfig);
    m_p2pService = nodeP2PServiceFactory.create();
}

int P2PServiceInitializer::init()
{
    // 忽略SIGPIPE信号，防止向一个已经断开的socket发送数据时操作系统触发SIGPIPE信号退出该应用
    signal(SIGPIPE, SIG_IGN);

    m_p2pService->registerModulePacketHandler(protocol::ModuleID::rpc,
        [&](const NodeId& nodeId, const std::vector<char>& data) -> int {
            packetprocess::PacketRawString packetRawString;
            packetRawString.decode(data.data(), data.size());

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

    return m_p2pService->init();
}

int P2PServiceInitializer::start()
{
    return m_p2pService->start();
}

int P2PServiceInitializer::stop()
{
    return m_p2pService->stop();
}

csm::service::P2PService* P2PServiceInitializer::p2pService()
{
    return m_p2pService.get();
}