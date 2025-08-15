#include<iostream>
#include<string>
#include "friend.pb.h"
#include "mprpcapplication.h"
#include "mprpcprovider.h"
#include "logger.h"

class FriendService:public fixbug::FriendServiceRpc
{
public:
    std::vector<std::string> GetFriendsList(uint32_t userid)
    {
        std::cout<<"do GetFriendsList service!"<<std::endl;
        std::vector<std::string> vec;
        vec.push_back("Tom");
        vec.push_back("Peter");
        vec.push_back("Anny");
        return vec;
    }

    void GetFriendList(::google::protobuf::RpcController* controller,
        const ::fixbug::GetFriendListRequest* request,
        ::fixbug::GetFriendListResponse* response,
        ::google::protobuf::Closure* done) override
    {
        //1.获取参数
        uint32_t userid=request->id();
        
        //2.调用本地方法
        std::vector<std::string> friendList=GetFriendsList(userid);

        //3.设置返回值
        for(auto &it:friendList)
        {
            response->add_friends(it);
        }

        //4.调用回调函数
        done->Run();
    }
};

int main(int argc,char** argv)
{
    LOG_INFO("first log message!");
    LOG_ERR("%s:%s:%d",__FILE__,__FUNCTION__,__LINE__);

    //调用框架的初始化操作：provider -i config.conf
    MprpcApplication::Init(argc,argv);

    //provider是一个rpc网络服务对象。把UserService对象发布到rpc节点上
    RpcProvider provider;
    provider.NotifyService(new FriendService());

    //启动一个rpc服务发布节点，Run以后，进程进入阻塞状态，等待远程的rpc调用请求
    provider.Run();

    return 0;
}