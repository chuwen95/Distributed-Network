#include <iostream>

#include <signal.h>

#include "libservice/TcpService.h"
#include "libpacketprocess/PacketProcessor.h"

class ExitHandler
{
public:
    static void exitHandler(int signal)
    {
        ExitHandler::c_shouldExit = true;
    }

    bool shouldExit() const { return ExitHandler::c_shouldExit.load(); }

    static std::atomic_bool c_shouldExit;
};

std::atomic_bool ExitHandler::c_shouldExit{false};

int main(int argc, char* argv[])
{
    if(argc < 2)
    {
        std::cerr << "usage: ./TcpServer config" << std::endl;
        std::cerr << "eg: ./TcpServer config.ini" << std::endl;
    }


    // 忽略SIGPIPE信号，防止向一个已经断开的socket发送数据时操作系统触发SIGPIPE信号退出该应用
    signal(SIGPIPE, SIG_IGN);

    packetprocess::PacketProcessor packetProcessor;

    service::TcpService tcpServer;
    tcpServer.registerPacketHandler([&packetProcessor](const packetprocess::PacketType packetType, packetprocess::PacketBase::Ptr packet,packetprocess::PacketReplyBase::Ptr replyPacket){
        return packetProcessor.process(packetType, packet, replyPacket);
    });
    if(-1 == tcpServer.init(argv[1]))
    {
        return -1;
    }
    tcpServer.start();

    ExitHandler exitHandler;
    signal(SIGTERM, &ExitHandler::exitHandler);
    signal(SIGABRT, &ExitHandler::exitHandler);
    signal(SIGINT, &ExitHandler::exitHandler);

    while(false == exitHandler.shouldExit())
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }

    tcpServer.stop();
    tcpServer.uninit();

    return 0;
}
