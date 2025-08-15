#pragma once
#include "google/protobuf/service.h"
#include<muduo/net/TcpServer.h>
#include<muduo/net/EventLoop.h>
#include<string.h>
#include<unordered_map>
#include<functional>
#include<google/protobuf/descriptor.h>

//框架提供的专门用于发布rpc服务的类
class RpcProvider{
public:
    //框架提供给外部使用的，可以发布rpc方法的函数接口
    void NotifyService(google::protobuf::Service* service); //UserService继承UserServiceRpc，UserServiceRpc继承google::protobuf::Service
    
    //启动rpc服务节点，开始提供rpc远程网络调用服务
    void Run();
private:
    muduo::net::EventLoop m_loop;

    //service服务信息结构体：服务指针+其下方法
    struct ServiceInfo{
        google::protobuf::Service* service;
        //存储服务下的方法
        std::unordered_map<std::string,const google::protobuf::MethodDescriptor*> m_methodMap;
    };
    //存储注册成功的服务对象和其服务方法的所有信息
    std::unordered_map<std::string,ServiceInfo> m_serviceInfoMap;


    //连接时的回调函数
    void onConnection(const muduo::net::TcpConnectionPtr&);

    //接收信息时的回调函数
    void onMessage(const muduo::net::TcpConnectionPtr&,muduo::net::Buffer*,muduo::Timestamp);

    //Closure的回调操作，用于序列化rpc的响应和网络发送
    void SendRpcResponse(const muduo::net::TcpConnectionPtr&,google::protobuf::Message*);
};