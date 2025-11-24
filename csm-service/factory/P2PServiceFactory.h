//
// Created by ChuWen on 10/11/23.
//

#ifndef COPYSTATEMACHINE_P2PSERVICEFACTORY_H
#define COPYSTATEMACHINE_P2PSERVICEFACTORY_H

#include "csm-tool/NodeConfig.h"
#include "csm-service/service/P2PService.h"

namespace csm
{

    namespace service
    {

        class P2PServiceFactory
        {
        public:
            explicit P2PServiceFactory(tool::NodeConfig* nodeConfig);

        public:
            std::unique_ptr<P2PService> create();

        private:
            tool::NodeConfig* m_nodeConfig;
            // 创建数据解码器
            std::unique_ptr<SessionDataDecoder> createSessionDataDecoder(std::size_t workerNum);
            // 创建网络模块包处理器
            std::unique_ptr<SessionServiceDataProcessor> createServiceDataProcessor();
            // 创建其他模组包处理器
            std::unique_ptr<SessionModuleDataProcessor> createModuleDataProcessor(std::size_t workerNum);
            // 创建路由选择算法 - 距离矢量
            std::unique_ptr<DistanceVector> createDistanceVector();
        };

    } // service

}

#endif //COPYSTATEMACHINE_NODEP2PSERVICEFACTORY_H
