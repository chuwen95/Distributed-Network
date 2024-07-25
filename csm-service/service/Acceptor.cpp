//
// Created by root on 9/4/23.
//

#include "Acceptor.h"

#include "csm-common/Common.h"
#include "csm-utilities/Logger.h"
#include "TcpSessionFactory.h"

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

                TcpSession::Ptr tcpSession = TcpSessionFactory().createTcpSession(clientfd, 4 * 1024, 4 * 1024);
                tcpSession->init();
                LOG->write(utilities::LogType::Log_Info, FILE_INFO, "create TcpSession successfully");

                if (nullptr != m_newClientCallback)
                {
                    LOG->write(utilities::LogType::Log_Info, FILE_INFO, "send TcpSession to SlaveReactorManager");
                    m_newClientCallback(clientfd, tcpSession);
                }
            }
        } while (-1 != clientfd);
    };
    m_thread.init(expression, 0, "acceptor");

    return 0;
}

int Acceptor::uninit()
{
    return 0;
}

int Acceptor::start()
{
    m_thread.start();

    return 0;
}

int Acceptor::stop()
{
    m_isTerminate = true;
    m_connectCv.notify_one();
    m_thread.stop();

    return 0;
}

int Acceptor::onConnect()
{
    LOG->write(utilities::LogType::Log_Trace, FILE_INFO);
    m_connectCv.notify_one();

    return 0;
}

void Acceptor::setNewClientCallback(std::function<void(int, TcpSession::Ptr)> newClientCallback)
{
    m_newClientCallback = newClientCallback;
}