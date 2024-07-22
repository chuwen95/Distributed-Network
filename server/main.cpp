#include <iostream>

#include "csm-initializer/Initializer.h"
#include "csm-packetprocess/packet/PacketRawString.h"

//#define FOR_TEST

class ExitHandler
{
public:
    static void exitHandler(int signal)
    {
        std::cout << "reveive signal " << signal << std::endl;
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
        std::cerr << "usage: ./TcpServer configFile" << std::endl;
        std::cerr << "eg: ./TcpServer config.ini" << std::endl;
        return -1;
    }

    csm::initializer::Initializer initializer;
    if(-1 == initializer.initConfig(argv[1]))
    {
        return -1;
    }
    if(-1 == initializer.init())
    {
        return -1;
    }
    if(-1 == initializer.start())
    {
        return -1;
    }
#ifdef FOR_TEST
    std::this_thread::sleep_for(std::chrono::seconds(3));

    packetprocess::PacketRawString packetRawString;
    packetRawString.setContent("hello world");

    std::shared_ptr<std::vector<char>> data = std::make_shared<std::vector<char>>();
    data->resize(packetRawString.packetLength());
    packetRawString.encode(data->data(), data->size());
#endif

    ExitHandler exitHandler;
    signal(SIGTERM, &ExitHandler::exitHandler);
    signal(SIGABRT, &ExitHandler::exitHandler);
    signal(SIGINT, &ExitHandler::exitHandler);

    while(false == exitHandler.shouldExit())
    {
#ifdef FOR_TEST
        initializer.m_tcpServiceInitializer->tcpService()->boardcastModuleMessage(1, data);
        //std::this_thread::sleep_for(std::chrono::milliseconds(1));
#else
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
#endif
    }

    initializer.stop();
    initializer.uninit();

    return 0;
}
