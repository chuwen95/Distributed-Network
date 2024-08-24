//
// Created by ChuWen on 9/6/23.
//

#include "Socket.h"

#include "csm-common/Common.h"
#include "csm-utilities/Logger.h"

using namespace csm::utilities;

Socket::Socket()
{}

Socket::~Socket()
{}

int Socket::create()
{
    int fd = socket(AF_INET, SOCK_STREAM, 0);
    if (-1 == fd)
    {
        LOG->write(utilities::LogType::Log_Error, FILE_INFO, "create socket failed, ", "errno: ", errno, ", ", strerror(errno));
        return -1;
    }

    return fd;
}

int Socket::close(const int fd)
{
    ::close(fd);
    return 0;
}

int Socket::setNonBlock(const int fd)
{
    int flags = fcntl(fd, F_GETFL, 0);
    if (-1 == flags)
    {
        LOG->write(utilities::LogType::Log_Error, FILE_INFO, "get socket flags failed, ", "errno: ", errno, ", ", strerror(errno));
        return -1;
    }
    if (-1 == fcntl(fd, F_SETFL, flags | O_NONBLOCK))
    {
        LOG->write(utilities::LogType::Log_Error, FILE_INFO, "set socket nonblock failed, ", "errno: ", errno, ", ", strerror(errno));
        return -1;
    }
    return 0;
}

int Socket::bind(const int fd, const std::string_view ip, const unsigned int port)
{
    struct sockaddr_in servAddr;
    memset(&servAddr, 0, sizeof(servAddr));
    servAddr.sin_family = AF_INET;
    servAddr.sin_port = htons(port);
    if (-1 == inet_pton(AF_INET, std::string(ip).c_str(), &(servAddr.sin_addr)))
    {
        LOG->write(utilities::LogType::Log_Error, FILE_INFO, "inet_pton ip address failed, ", "errno: ", errno, ", ", strerror(errno));
        return -1;
    }

    if (-1 == ::bind(fd, (struct sockaddr *) &servAddr, sizeof(servAddr)))
    {
        LOG->write(utilities::LogType::Log_Error, FILE_INFO, "bind ip and port failed, ", "errno: ", errno, ", ", strerror(errno));
        return -1;
    }
    return 0;
}

int Socket::listen(const int fd, const std::size_t conNum)
{
    if (-1 == ::listen(fd, conNum))
    {
        LOG->write(utilities::LogType::Log_Error, FILE_INFO, "listen failed, ", "errno: ", errno, ", ", strerror(errno));
    }
    return 0;
}

int Socket::connect(const int fd, const std::string &ip, const unsigned short port)
{
    struct sockaddr_in serverAddr;
    memset(&serverAddr, 0, sizeof(serverAddr));
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(port);
    inet_pton(AF_INET, ip.c_str(), &(serverAddr.sin_addr.s_addr));

    return ::connect(fd, (struct sockaddr *) &serverAddr, sizeof(serverAddr));
}

int Socket::getSocketKernelRecvBufferSize(const int fd)
{
    int recvBufSize{ -1 };

    socklen_t optlen;
    if(-1 == getsockopt(fd, SOL_SOCKET, SO_RCVBUF, &recvBufSize, &optlen))
    {
        LOG->write(utilities::LogType::Log_Error, FILE_INFO, "get socket recv buffer failed, ", "errno: ", errno, ", ", strerror(errno));
        return -1;
    }

    return 0;
}

int Socket::setSocketKernelRecvBufferSize(const int fd, const int size)
{
    if(-1 == setsockopt(fd, SOL_SOCKET, SO_RCVBUF, &size, sizeof(size)))
    {
        LOG->write(utilities::LogType::Log_Error, FILE_INFO, "set socket recv buffer failed, ", "errno: ", errno, ", ", strerror(errno));
        return -1;
    }

    return 0;
}