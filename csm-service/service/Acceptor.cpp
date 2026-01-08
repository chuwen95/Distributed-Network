//
// Created by ChuWen on 9/4/23.
//

#include "Acceptor.h"

#include <google/protobuf/message.h>

#include "P2PSessionFactory.h"
#include "csm-common/Common.h"
#include "csm-utilities/Logger.h"
#include "csm-utilities/Socket.h"

using namespace csm::service;

Acceptor::Acceptor(std::shared_ptr<P2PSessionFactory> p2pSessionFactory) : m_p2pSessionFactory(
    std::move(p2pSessionFactory))
{
}

int Acceptor::init(const int fd)
{
    const auto expression = [this, listenfd = fd](std::stop_token st)
    {
        {
            std::unique_lock<std::mutex> ulock(x_connect);
            m_connectCv.wait(ulock);
        }

        if (true == st.stop_requested())
        {
            return;
        }

        int connfd{-1};
        do
        {
            struct sockaddr_in clientAddr;
            memset(&clientAddr, 0, sizeof(clientAddr));
            socklen_t len = sizeof(clientAddr);
            connfd = accept(listenfd, (struct sockaddr*)&clientAddr, &len);
            if (connfd < 0)
            {
                if (errno == EAGAIN)
                {
                    continue;
                }
                else
                {
                    LOG->write(utilities::LogType::Log_Error, FILE_INFO, "accept connection failed, errno: ", errno,
                               strerror(errno));
                    continue;
                }
            }

            LOG->write(utilities::LogType::Log_Info, FILE_INFO, "client online, fd: ", connfd);

            // 设置socket接收缓冲区大小
            if (-1 == utilities::Socket::setSocketKernelRecvBufferSize(
                connfd, utilities::Socket::c_defaultSocketRecvBufferSize))
            {
                LOG->write(utilities::LogType::Log_Error, FILE_INFO, "set socket recv buffer size failed, errno: ",
                           errno,
                           ", ", strerror(errno));
                utilities::Socket::close(connfd);
                continue;
            }
            LOG->write(utilities::LogType::Log_Debug, FILE_INFO, "set socket recv buffer size to ",
                       utilities::Socket::c_defaultSocketRecvBufferSize, " successfully, fd: ", connfd);

            P2PSession::Ptr p2pSession = m_p2pSessionFactory->create(connfd, c_p2pSessionReadBufferSize,
                                                                     c_p2pSessionWriteBufferSize);
            p2pSession->init();
            LOG->write(utilities::LogType::Log_Info, FILE_INFO, "create P2PSession successfully, fd: ", connfd,
                       ", session id: ", p2pSession->sessionId());

            if (nullptr != m_newClientCallback)
            {
                LOG->write(utilities::LogType::Log_Info, FILE_INFO, "send P2PSession to SessionDispatcher, fd: ",
                           connfd,
                           ", session id: ",
                           p2pSession->sessionId());
                m_newClientCallback(p2pSession);
            }
        }
        while (connfd > 0 && false == st.stop_requested());
    };
    m_thread = std::make_unique<utilities::Thread>(expression, 0, "acceptor");

    return 0;
}

int Acceptor::start()
{
    m_thread->start();

    return 0;
}

int Acceptor::stop()
{
    m_thread->stop();
    m_connectCv.notify_one();

    return 0;
}

void Acceptor::onConnect()
{
    m_connectCv.notify_one();
}

void Acceptor::setNewClientCallback(std::function<void(P2PSession::Ptr p2pSession)> newClientCallback)
{
    m_newClientCallback = std::move(newClientCallback);
}
