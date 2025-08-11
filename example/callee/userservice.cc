#include<iostream>
#include<string>
#include "user.pb.h"
#include "mprpcapplication.h"
#include "rpcprovider.h"

class UserService:public fixbug::UserServiceRpc  //使用在rpc服务发布端（rpc服务提供者） 
{
public:
    bool login(std::string name,std::string pwd)
    {
        std::cout<<"name:"<<name<<" "<<"pwd:"<<pwd<<"登录成功"<<std::endl;
        return true;
    }

    /*
    重写基类UserServiceRpc的虚函数，下面这些方法都是有框架直接调用的（注：当前是业务层）
    1.caller(rpc服务调用者) ===> 调用Login(LoginRequest) ===> 通过muduo网络库 ===>发送到 callee(rpc服务提供者)
    2.callee(rpc服务提供者) ===> 接收Login(LoginRequest)请求 ===> 交到下面重写的这个Login方法上了
    */

    void Login(::google::protobuf::RpcController* controller,
        const ::fixbug::LoginRequest* request,
        ::fixbug::LoginResponse* response,
        ::google::protobuf::Closure* done)
    {
        //LoginRequest、LoginResponse的序列化与反序列话都由框架完成
        //框架给业务上报了请求参数 LoginRequest，应用获取相应数据做本地服务
        std::string name=request->name();
        std::string pwd=request->pwd();

        //做本地业务
        bool login_result=login(name,pwd);
        
        //把响应写入LoginResponse （错误码、错误消息、返回值）
        fixbug::ResultCode* result_code=response->mutable_result();
        result_code->set_code(0);
        result_code->set_errmsg("");
        response->set_success(login_result);

        //调用done回调函数，通知框架本次rpc调用已经处理完毕
        //框架会在done回调函数中，发送LoginResponse响应给rpc服务调用者
        done->Run();
    }
};

int main(int argc,char** argv)
{
    //调用框架的初始化操作：provider -i config.conf
    MprpcApplication::Init(argc,argv);

    //provider是一个rpc网络服务对象。把UserService对象发布到rpc节点上
    RpcProvider provider;
    provider.NotifyService(new UserService());

    //启动一个rpc服务发布节点，Run以后，进程进入阻塞状态，等待远程的rpc调用请求
    provider.Run();

    return 0;
}