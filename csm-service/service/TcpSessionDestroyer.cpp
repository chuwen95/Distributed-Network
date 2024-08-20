//
// Created by ChuWen on 2024/8/8.
//

#include "TcpSessionDestroyer.h"

using namespace csm::service;

TcpSessionDestroyer::TcpSessionDestroyer(TcpSessionManager::Ptr tcpSessionManager, HostsConnector::Ptr hostsConnector,
                                         HostsInfoManager::Ptr hostsInfoManager) :
                                         m_tcpSessionManager(std::move(tcpSessionManager)), m_hostsConnector(std::move(hostsConnector)),
                                         m_hostsInfoManager(std::move(hostsInfoManager))
{
}
