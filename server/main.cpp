#include <iostream>

#include "libinitializer/Initializer.h"

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

    initializer::Initializer initializer;
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

    ExitHandler exitHandler;
    signal(SIGTERM, &ExitHandler::exitHandler);
    signal(SIGABRT, &ExitHandler::exitHandler);
    signal(SIGINT, &ExitHandler::exitHandler);

    while(false == exitHandler.shouldExit())
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }

    initializer.stop();
    initializer.uninit();

    return 0;
}
