//
// Created by ChuWen on 10/11/23.
//

#ifndef P2PSERVICEFACTORY_H
#define P2PSERVICEFACTORY_H

#include "config/ServiceConfig.h"
#include "P2PService.h"

namespace csm
{

    namespace service
    {

        class P2PServiceFactory
        {
        public:
            using Ptr = std::shared_ptr<P2PServiceFactory>;

            P2PServiceFactory(tool::NodeConfig::Ptr nodeConfig, ServiceStartType serviceStartType);
            ~P2PServiceFactory() = default;

        public:
            P2PService::Ptr create();

        private:
            SessionDataDecoder::Ptr createSessionDataDecoder(
                P2PSessionManager::Ptr p2pSessionManager, std::size_t workerNum);
            SessionServiceDataProcessor::Ptr createServiceDataProcessor();
            SessionModuleDataProcessor::Ptr createModuleDataProcessor(
                P2PSessionManager::Ptr p2pSessionManager, std::size_t workerNum);

        private:
            tool::NodeConfig::Ptr m_nodeConfig;
            ServiceStartType m_serviceStartType;
        };

    } // service

}

#endif //P2PSERVICEFACTORY_H
