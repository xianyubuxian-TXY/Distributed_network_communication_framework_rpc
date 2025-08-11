#include "rpcprovider.h"
#include "mprpcapplication.h"
#include<string.h>
#include<functional>
//框架提供给外部使用的，可以发布rpc方法的函数接口
void RpcProvider::NotifyService(google::protobuf::Service* service) //UserService继承UserServiceRpc，UserServiceRpc继承google::protobuf::Service
{

}

//启动rpc服务节点，开始提供rpc远程网络调用服务
void RpcProvider::Run()
{
    std::string ip=MprpcApplication::GetInstance().getConfig().Load("rpcserverip");
    uint16_t port=atoi(MprpcApplication::GetInstance().getConfig().Load("rpcserverport").c_str());

    muduo::net::InetAddress addr(ip,port);
    muduo::net::TcpServer m_tcpServer(&m_loop,addr,"RpcProvider");

    //设置连接/断开时的回调函数
    m_tcpServer.setConnectionCallback(std::bind(&RpcProvider::onConnection,this,std::placeholders::_1));
    //设置发送/接收消息时的回调函数
    m_tcpServer.setMessageCallback(std::bind(&RpcProvider::onMessage,this,std::placeholders::_1,std::placeholders::_2,std::placeholders::_3));

    //设置开启的线程数
    m_tcpServer.setThreadNum(4);

    std::cout<<"RpcProvider start service at ip:"<<ip<<" port:"<<port<<std::endl;

    //启动网络服务
    m_tcpServer.start();
    m_loop.loop();
}

//连接时的回调函数
void RpcProvider::onConnection(const muduo::net::TcpConnectionPtr&)
{

}

//接收信息时的回调函数
void RpcProvider::onMessage(const muduo::net::TcpConnectionPtr&,muduo::net::Buffer*,muduo::Timestamp)
{

}