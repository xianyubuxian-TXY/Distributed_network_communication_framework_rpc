#include "mprpcchannel.h"
#include "rpcheader.pb.h"
#include "mprpcapplication.h"
#include "mprpccontroller.h"
#include "zookeeperutil.h"
/*
header_size+header_str(service_name method_name args_size)+args_str
*/

//短连接
void MprpcChannel::CallMethod(const google::protobuf::MethodDescriptor* method,
                            google::protobuf::RpcController* controller, 
                            const google::protobuf::Message* request, //参数
                            google::protobuf::Message* response, //返回值
                            google::protobuf::Closure* done)
{
    //获取服务名和方法名
    const google::protobuf::ServiceDescriptor *service=method->service();
    std::string service_name=service->name();
    std::string method_name=method->name();

    //序列化参数,并获取序列化后的参数长度
    std::string args_str;
    uint32_t args_size=0;
    if(request->SerializeToString(&args_str))
    {
        args_size=args_str.size();
    }
    else
    {
        //设置控制信息
        controller->SetFailed("serialize request error!");
        return;
    }

    //定义rpc的请求头RpcHeader
    mprpc::RpcHeader rpcHeader;
    rpcHeader.set_service_name(service_name);
    rpcHeader.set_method_name(method_name);
    rpcHeader.set_args_size(args_size);

    //序列化RpcHeader,并获取header_size
    std::string header_str;
    uint32_t header_size=0;
    if(rpcHeader.SerializeToString(&header_str))
    {
        header_size=header_str.size();
    }
    else
    {
        controller->SetFailed("serialize header error!");
        return;
    }

    //组织待发送的rpc请求的字符串（注意：header_size占前4个字节）
    std::string send_rpc_str;
    send_rpc_str.insert(0,std::string((char*)&header_size,4));
    send_rpc_str+=header_str; //拼接header_str
    send_rpc_str+=args_str; //凭借args_str
    
    //打印调试信息
    std::cout<<"================================================="<<std::endl;
    std::cout<<"header_size:"<<header_size<<std::endl;
    std::cout<<"header_str:"<<header_str<<std::endl;
    std::cout<<"service_name:"<<service_name<<std::endl;
    std::cout<<"method_name:"<<method_name<<std::endl;
    std::cout<<"args_size:"<<args_size<<std::endl;
    std::cout<<"args_str:"<<args_str<<std::endl;
    std::cout<<"================================================="<<std::endl;
    
    //客户端不需要高并发，故使用tcp编程完成rpc方法的远程调用
    int clientfd=socket(AF_INET,SOCK_STREAM,0);
    if(-1==clientfd)
    {
        controller->SetFailed("create socket error! errno:"+std::to_string(errno));
        return;
    }

    //通过MprpcApplication获取配置文件中server的ip，port
    //std::string ip=MprpcApplication::getConfig().Load("rpcserverip");
    //uint16_t port=atoi(MprpcApplication::getConfig().Load("rpcserverport").c_str());

    //rpc调用方想调用service_name的method_name服务，需要查询zk上该服务所在的host信息
    ZkClient zkCli;
    zkCli.Start();
    std::string method_path="/"+service_name+"/"+method_name;
    //127.0.0.1:8000
    std::string host_data=zkCli.GetData(method_path.c_str());
    if(host_data=="")
    {
        controller->SetFailed(method_path+" is not exist!");
        return;
    }
    int idx=host_data.find(":");
    if(idx==-1)
    {
        controller->SetFailed(method_path+" address is invalied!");
    }
    std::string ip=host_data.substr(0,idx);
    uint16_t port=atoi(host_data.substr(idx+1).c_str());

    sockaddr_in addr;
    memset(&addr,0,sizeof(addr));
    addr.sin_family=AF_INET;
    addr.sin_addr.s_addr=inet_addr(ip.c_str());
    addr.sin_port=htons(port);
    if(-1==connect(clientfd,(sockaddr*)&addr,sizeof(addr)))
    {
        controller->SetFailed("connect error! errno:"+std::to_string(errno));
        close(clientfd);
        return;
    }

    if(-1==send(clientfd,send_rpc_str.c_str(),send_rpc_str.size(),0))
    {
        controller->SetFailed("send error! errno:"+std::to_string(errno));
        close(clientfd);
        return;
    }

    char buffer[1024]={0};
    int recv_size=0;
    if(-1==(recv_size=recv(clientfd,buffer,sizeof(buffer),0)))
    {
        controller->SetFailed("recv error! errno:"+std::to_string(errno));
        close(clientfd);
        return;
    }

    //反序列化arp调用的响应数据，并存入response
    std::string response_str(buffer,recv_size);
    if(!response->ParseFromString(response_str))
    {
        controller->SetFailed("parse error! response_str:");
        close(clientfd);
        return;
    }

    close(clientfd); //短连接
}