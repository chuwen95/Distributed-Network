# TcpNetwork

## 基本特性
1. 网络模块使用reactor模式实现
2. server/client一体，可实现节点互联，节点互联仅保留一条连接

## 更多文档
1. [网络模块关键点介绍及UML类图](./resources/network_module_instruction/网络模块关键点解析.md)
2. [关于处理重复连接的握手协议介绍](resources/two_node_one_connection/关于处理重复连接的握手协议介绍.md)

## Todo
1. SlaveReactor发送线程使用锁太多，客户端掉线逻辑修改为专门的类中线程处理，send或者recv一旦返回客户端掉线，那么马上从epoll中移除，并加入到掉线处理类列表中，异步处理掉线，将fd和TcpSession的关系放在单独的类中，传入SlaveReactor中
2. SlaveReactor发送，先尝试send，如果send失败，则EPOLL_CTL_ADD EPOLL_OUT事件
3. 读写当作一个任务，收到EPOLL_IN/EPOLL_OUT后，将读写任务放入线程池中，这样不必担心数据到来速度大于recv速度导致recv返回值不会小于-1，可以一直recv从而阻塞数据发送等后续逻辑的情况