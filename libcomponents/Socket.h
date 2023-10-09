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

        /**
         * @brief 设置socket选项SO_NOSIGPIPE，避免对端断开连接后send因SIGPIPE信号崩溃
         *              设置为此选项后，若对端断开，send返回-1，errno为EPIPE(Broken pipe)
         *
         * @param fd
         * @return
         */
        static int setNoSigPipe(const int fd);

        static int connect(const int fd, const std::string& ip, const unsigned short port);
    };

} // componnets

#endif //TCPSERVER_SOCKET_H
