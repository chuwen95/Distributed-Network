# TcpNetwork

## 基本特性
1. 网络模块使用reactor模式实现，位于csm-service模块
2. server/client一体，通过握手协议实现节点互联仅保留一条连接
3. 可仅启动server，作为tcp rpc的服务端

## 更多文档
1. [网络模块关键点介绍及UML类图](./resources/network_module_instruction/网络模块关键点解析.md)
2. [关于处理重复连接的握手协议介绍](./resources/two_node_one_connection/关于处理重复连接的握手协议介绍.md)

## Todo
1. 增加路由表，优化节点间点对点通信效率
2. 增加树形拓扑，优化节点消息广播效率