//
// Created by root on 10/21/23.
//

#include "httplib.h"
#include "csm-utilities/StringTool.h"

int main(int argc, char* argv[])
{
    if(3 != argc)
    {
        std::cout << "param error, format: ./HttpClient serverIp serverPort" << std::endl;
        std::cout << "eg: ./HttpClient 127.0.0.1 20200" << std::endl;
    }

    std::string content{"hello world"};

    httplib::Client client(argv[1], csm::utilities::convertFromString<int>(argv[2]));
    while(true)
    {
        client.Post("/boardcastRawString", content.c_str(), content.size(), "text/plain");
        //std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }

    return 0;
}