#include <iostream>

#include <signal.h>

#include "libreactor/TcpServer.h"
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
        std::cerr << "usage: ./TcpServer logLevel(trace/debug/info/warning/error)" << std::endl;
        std::cerr << "eg: ./TcpServer debug" << std::endl;
    }

    int logLevel{3};
    if("trace" == std::string(argv[1]))
    {
        logLevel = 0;
    }
    else if("debug" == std::string(argv[1]))
    {
        logLevel = 1;
    }
    else if("info" == std::string(argv[1]))
    {
        logLevel = 2;
    }
    else if("warning" == std::string(argv[1]))
    {
        logLevel = 3;
    }
    else if("error" == std::string(argv[1]))
    {
        logLevel = 4;
    }
    else
    {
        std::cout << "param error" << std::endl;
        return -1;
    }

    // 忽略SIGPIPE信号，防止向一个已经断开的socket发送数据时操作系统触发SIGPIPE信号退出该应用
    signal(SIGPIPE, SIG_IGN);

    packetprocess::PacketProcessor packetProcessor;

    server::TcpServer tcpServer;
    tcpServer.setLogLevel(logLevel);
    tcpServer.registerPacketHandler([&packetProcessor](const packetprocess::PacketType packetType, packetprocess::PacketBase::Ptr packet,packetprocess::PacketReplyBase::Ptr replyPacket){
        return packetProcessor.process(packetType, packet, replyPacket);
    });
    tcpServer.init("127.0.0.1", 9999, "/data/server_log.log");
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
