#include<iostream>
#include "friend.pb.h"
#include "mprpcapplication.h"

int main(int argc,char** argv)
{
    //整个程序启动后，想要使用mprpc框架来享受rpc服务调用，一定需要先调用框架的初始化函数（只出初始化一个）
    MprpcApplication::Init(argc,argv);

    //调用方使用 xxx_Stub来调用方法： 而每个方法都是对”通过RpcChannel* 调用CallMethod()方法“
    //RpcChannel是一个抽象基类，rpc框架可通过继承该类，来自定义CallMethod方法
    //MprpcChannel是我们实现的rpc框架继承自RpcChannel类的类型
    
    //1.演示调用远程发布的rpc发送Login业务
    //定义Stub对象
    fixbug::FriendServiceRpc_Stub stub(new MprpcChannel());
    //rpc方法的请求参数
    fixbug::GetFriendListRequest request;
    request.set_id(1);
    //rpc方法的响应
    fixbug::GetFriendListResponse response;
    //发起rpc方法的调用，同步的rpc调用过程 MprpcChannel::callmethod
    MprpcController controller; //获取stub调用方法的状态信息（成功？失败？）
    stub.GetFriendList(&controller,&request,&response,nullptr); //RpcChannel->RpcChannel::callMethod 集中来中所有的rpc方法调用的参数序列化和网络发送
    
    if(controller.Failed())
    {
        //调用远程发布的rpc发送业务失败
        std::cout<<"rpc failed"<<std::endl;
        std::cout<<controller.ErrorText()<<std::endl;
    }
    else
    {
        //一次rpc调用完成，读调用的结果
        if(0==response.result().errcode())
        {
            std::cout<<"rpc login response success"<<std::endl;
            int size=response.friends_size();
            for(int i=0;i<size;++i)
            {
                std::cout<<"index:"<<(i+1)<<" namme:"<<response.friends(i)<<std::endl;
            }
        }
        else
        {
            std::cout<<"rpc login response error:"<<response.result().errmsg()<<std::endl;
        }
    }

    return 0;
}