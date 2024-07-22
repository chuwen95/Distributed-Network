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

    httplib::Server httpServer;
    httpServer.Post("/boardcastRawString", [](const httplib::Request& req, httplib::Response& res){
        std::cout << req.body << std::endl;
    });
    httpServer.listen(argv[1], csm::utilities::convertFromString<int>(argv[2]));

    return 0;
}