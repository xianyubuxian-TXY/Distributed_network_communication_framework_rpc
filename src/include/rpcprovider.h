#pragma once
#include "google/protobuf/service.h"
#include<muduo/net/TcpServer.h>
#include<muduo/net/EventLoop.h>

//框架提供的专门用于发布rpc服务的类
class RpcProvider{
public:
    //框架提供给外部使用的，可以发布rpc方法的函数接口
    void NotifyService(google::protobuf::Service* service); //UserService继承UserServiceRpc，UserServiceRpc继承google::protobuf::Service
    
    //启动rpc服务节点，开始提供rpc远程网络调用服务
    void Run();
private:
    muduo::net::EventLoop m_loop;

    //连接时的回调函数
    void onConnection(const muduo::net::TcpConnectionPtr&);

    //接收信息时的回调函数
    void onMessage(const muduo::net::TcpConnectionPtr&,muduo::net::Buffer*,muduo::Timestamp);
};