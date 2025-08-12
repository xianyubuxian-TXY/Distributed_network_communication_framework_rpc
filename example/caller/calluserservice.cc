#include<iostream>
#include "user.pb.h"
#include "mprpcapplication.h"
#include "mprpcchannel.h"

int main(int argc,char** argv)
{
    //整个程序启动后，想要使用mprpc框架来享受rpc服务调用，一定需要先调用框架的初始化函数（只出初始化一个）
    MprpcApplication::Init(argc,argv);

    //调用方使用 xxx_Stub来调用方法： 而每个方法都是对”通过RpcChannel* 调用CallMethod()方法“
    //RpcChannel是一个抽象基类，rpc框架可通过继承该类，来自定义CallMethod方法
    //MprpcChannel是我们实现的rpc框架继承自RpcChannel类的类型
    
    //演示调用远程发布的rpc发送Login
    fixbug::UserServiceRpc_Stub stub(new MprpcChannel());
    //rpc方法的请求参数
    fixbug::LoginRequest request;
    request.set_name("li si");
    request.set_pwd("123456");
    //rpc方法的响应
    fixbug::LoginResponse response;
    //发起rpc方法的调用，同步的rpc调用过程 MprpcChannel::callmethod
    stub.Login(nullptr,&request,&response,nullptr); //RpcChannel->RpcChannel::callMethod 集中来中所有的rpc方法调用的参数序列化和网络发送

    //一次rpc调用完成，读调用的结果
    if(0==response.result().errcode())
    {
        std::cout<<"rpc login response:"<<response.success()<<std::endl;
    }
    else
    {
        std::cout<<"rpc login response error:"<<response.result().errmsg()<<std::endl;
    }

    return 0;
}