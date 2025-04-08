//
// Created by ChuWen on 9/4/23.
//

#include "Acceptor.h"

#include <google/protobuf/message.h>

#include "csm-common/Common.h"
#include "csm-utilities/Logger.h"
#include "csm-utilities/Socket.h"
#include "P2PSessionFactory.h"

using namespace csm::service;

int Acceptor::init(const int fd)
{
    const auto expression = [this, listenfd = fd]() {
        if (true == m_isTerminate)
        {
            std::this_thread::sleep_for(std::chrono::milliseconds(5));
            return;
        }

        {
            std::unique_lock<std::mutex> ulock(x_connect);
            m_connectCv.wait(ulock);
        }


        if (true == m_isTerminate)
        {
            std::this_thread::sleep_for(std::chrono::milliseconds(5));
            return;
        }

        int clientfd{-1};
        do
        {
            struct sockaddr_in clientAddr;
            memset(&clientAddr, 0, sizeof(clientAddr));
            socklen_t len = sizeof(clientAddr);
            int clientfd = accept(listenfd, (struct sockaddr *) &clientAddr, &len);
            if (-1 != clientfd)
            {
                LOG->write(utilities::LogType::Log_Info, FILE_INFO, "client online, fd: ", clientfd);

                // 设置socket接收缓冲区大小
                if(-1 == utilities::Socket::setSocketKernelRecvBufferSize(clientfd, utilities::Socket::c_defaultSocketRecvBufferSize))
                {
                    LOG->write(utilities::LogType::Log_Error, FILE_INFO, "set socket recv buffer size failed, errno: ", errno, ", ", strerror(errno));
                    utilities::Socket::close(clientfd);
                    continue;
                }
                LOG->write(utilities::LogType::Log_Debug, FILE_INFO, "set socket recv buffer size to ", utilities::Socket::c_defaultSocketRecvBufferSize, " successfully, fd: ", clientfd);

                P2PSession::Ptr p2pSession = P2PSessionFactory().create(clientfd, c_p2pSessionReadBufferSize, c_p2pSessionWriteBufferSize);
                p2pSession->init();
                LOG->write(utilities::LogType::Log_Info, FILE_INFO, "create P2PSession successfully, fd: ", clientfd);

                if (nullptr != m_newClientCallback)
                {
                    LOG->write(utilities::LogType::Log_Info, FILE_INFO, "send P2PSession to SessionDispatcher, fd: ", clientfd);
                    m_newClientCallback(clientfd, p2pSession);
                }
            }
        } while (-1 != clientfd);
    };
    m_thread = std::make_shared<utilities::Thread>(expression, 0, "acceptor");

    return 0;
}

int Acceptor::start()
{
    m_thread->start();

    return 0;
}

int Acceptor::stop()
{
    m_isTerminate = true;
    m_connectCv.notify_one();
    m_thread->stop();

    return 0;
}

int Acceptor::onConnect()
{
    LOG->write(utilities::LogType::Log_Trace, FILE_INFO);
    m_connectCv.notify_one();

    return 0;
}

void Acceptor::setNewClientCallback(std::function<void(int, P2PSession::Ptr)> newClientCallback)
{
    m_newClientCallback = newClientCallback;
}