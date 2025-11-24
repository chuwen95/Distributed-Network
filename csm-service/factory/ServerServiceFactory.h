//
// Created by ChuWen on 10/11/23.
//

#ifndef COPYSTATEMACHINE_SERVERSERVICEFACTORY_H
#define COPYSTATEMACHINE_SERVERSERVICEFACTORY_H

#include "csm-service/service/ServerService.h"

namespace csm
{

    namespace service
    {

        class ServerServiceFactory
        {
        public:
            explicit ServerServiceFactory(tool::NodeConfig* nodeConfig);
            ~ServerServiceFactory() = default;

        public:
            std::unique_ptr<ServerService> create();

        private:
            tool::NodeConfig* m_nodeConfig;
            // 创建数据解码器
            std::unique_ptr<SessionDataDecoder> createSessionDataDecoder(std::size_t workerNum);
            // 创建网络模块包处理器
            std::unique_ptr<SessionServiceDataProcessor> createServiceDataProcessor();
            // 创建其他模组包处理器
            std::unique_ptr<SessionModuleDataProcessor> createModuleDataProcessor(std::size_t workerNum);
        };

    } // service

}

#endif //COPYSTATEMACHINE_SERVERSERVICEFACTORY_H
