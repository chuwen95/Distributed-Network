//
// Created by root on 10/19/23.
//

#include "Rpc.h"
#include "csm-utilities/Logger.h"
#include "csm-packetprocess/packet/PacketRawString.h"
#include "csm-utilities/CellTimestamp.h"

using namespace csm::rpc;

Rpc::Rpc(RpcConfig::Ptr rpcConfig) : m_rpcConfig(rpcConfig)
{
}

int Rpc::init()
{
    m_rpcConfig->httpServer()->Post("/boardcastRawString", [this](const httplib::Request &req, httplib::Response &res) {

        //LOG->write(utilities::LogType::Log_Info, FILE_INFO,
        //                                                             " recv raw string: : ", req.body);

        packetprocess::PacketRawString packetRawString;
        packetRawString.setContent(req.body);

        std::shared_ptr<std::vector<char>> data = std::make_shared<std::vector<char>>();
        data->resize(packetRawString.packetLength());
        packetRawString.encode(data->data(), data->size());

        m_rpcConfig->tcpService()->boardcastModuleMessage(1, data);
    });

    return 0;
}

int Rpc::uninit()
{
    return 0;
}

int Rpc::start()
{
    std::thread([this]() {
        m_rpcConfig->httpServer()->listen(m_rpcConfig->nodeConfig()->rpcIp(), m_rpcConfig->nodeConfig()->rpcPort());
    }).detach();

    return 0;
}

int Rpc::stop()
{
    m_rpcConfig->httpServer()->stop();

    return 0;
}