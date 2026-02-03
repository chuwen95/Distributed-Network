# Distributed Network

## 简介
1. 分布式网络学习项目，模块命名借鉴优秀的开源区块链项目fisco-bcos(https://github.com/FISCO-BCOS/FISCO-BCOS.git)

## 基本特性
1. 网络使用reactor主从模式实现，位于csm-service模块
2. server/client一体，通过握手协议实现节点互联仅保留一条连接
3. 可仅启动server，作为tcp rpc的服务端
4. 准备使用距离向量算法实现节点到节点定点包传输

## 更多文档（没有和代码同步更新）
1. [网络模块关键点介绍及UML类图](./resources/network_module_instruction/网络模块关键点解析.md)
2. [关于处理重复连接的握手协议介绍](./resources/two_node_one_connection/关于处理重复连接的握手协议介绍.md)

## Todo
1. 增加树形拓扑，优化节点消息广播效率