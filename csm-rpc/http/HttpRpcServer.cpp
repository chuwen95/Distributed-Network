//
// Created by Chu on 2024/9/4.
//

#include "HttpRpcServer.h"
#include "csm-utilities/Logger.h"
#include "csm-packetprocess/packet/PacketRawString.h"

using namespace csm::rpc;

HttpRpcServer::HttpRpcServer(RpcConfig::Ptr rpcConfig) : RpcServer(std::move(rpcConfig))
{
    m_httpServer = std::make_shared<httplib::Server>();
}

int HttpRpcServer::init()
{
    m_httpServer->Post("/boardcastRawString", [this](const httplib::Request &req, httplib::Response &res) {
        // LOG->write(utilities::LogType::Log_Info, FILE_INFO, "recv raw string: : ", req.body);

        packetprocess::PacketRawString packetRawString;
        packetRawString.setContent(req.body);

        std::shared_ptr<std::vector<char>> data = std::make_shared<std::vector<char>>();
        data->resize(packetRawString.packetLength());
        packetRawString.encode(data->data(), data->size());

        m_rpcConfig->tcpService()->boardcastModuleMessage(1, data);
    });

    return 0;
}

int HttpRpcServer::start()
{
    std::thread([this]() {
        m_httpServer->listen(m_rpcConfig->nodeConfig()->httpRpcIp(), m_rpcConfig->nodeConfig()->httpRpcPort());
    }).detach();

    return 0;
}

int HttpRpcServer::stop()
{
    m_httpServer->stop();

    return 0;
}