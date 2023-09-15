//
// Created by root on 9/6/23.
//

#ifndef TCPSERVER_SOCKET_H
#define TCPSERVER_SOCKET_H

#include "libcommon/Common.h"

namespace components
{

    class Socket
    {
    public:
        Socket();
        ~Socket();

        static int create();

        static int close(const int fd);

        static int setNonBlock(const int fd);

        static int bind(const int fd, const std::string_view ip, const unsigned int port);

        static int listen(const int fd, const std::size_t conNum);
    };

} // componnets

#endif //TCPSERVER_SOCKET_H
