#include "mprpcprovider.h"
#include "mprpcapplication.h"
#include "rpcheader.pb.h"
#include "logger.h"
#include "zookeeperutil.h"
//框架提供给外部使用的，可以发布rpc方法的函数接口
//建立“服务+方法”表，用户通过“服务名+方法名+参数”调用对应服务下的方法
void RpcProvider::NotifyService(google::protobuf::Service* service) //UserService继承UserServiceRpc，UserServiceRpc继承google::protobuf::Service
{
    //创建ServiceInfo
    ServiceInfo serviceInfo;
    serviceInfo.service=service;

    //获取“服务描述符”（包含了服务名、服务下的方法等）
    const google::protobuf::ServiceDescriptor* serviceDescPtr=service->GetDescriptor();
    //获取服务名
    std::string serviceName=serviceDescPtr->name();
    //获取服务下的方法数量
    int serviceCount=serviceDescPtr->method_count();
    
    //std::cout<<"service_name:"<<serviceName<<std::endl;
    LOG_INFO("service_name:%s",serviceName.c_str());


    //一次获取服务下的每个方法
    for(int i=0;i<serviceCount;++i)
    {
        //获取方法指针
        const google::protobuf::MethodDescriptor* methodDescPtr=serviceDescPtr->method(i);
        //获取方法名
        std::string methodName=methodDescPtr->name();
        //填入ServiceInfo中的methodMap
        serviceInfo.m_methodMap.insert({methodName,methodDescPtr});
        LOG_INFO("method_name:%s",methodName.c_str());
    }

    //将ServiceInfo插入serviceMap
    m_serviceInfoMap.insert({serviceName,serviceInfo});
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

    //把当前rpc节点上要发布的服务全部注册到zk上面，让rpc client可以从zk上发现服务
    //session timeout  30s    zkclient 网络I/O  1/3*timeout时间发送ping消息
    ZkClient zkCli;
    zkCli.Start();
    //server_name为永久节点  method_name为临时性节点
    for(auto &sp:m_serviceInfoMap)
    {
        // "/service_name"
        std::string service_path="/"+sp.first;
        zkCli.Create(service_path.c_str(),nullptr,0);
        for(auto &mp:sp.second.m_methodMap)
        {
            // "/service_name/method_name"
            std::string method_path=service_path+"/"+mp.first;
            //存储的数据 “ip:port”
            char method_path_data[128]={0};
            sprintf(method_path_data,"%s:%d",ip.c_str(),port);
            // ZOO_EPHEMERAL表示znode是一个临时性节点
            zkCli.Create(method_path.c_str(),method_path_data,strlen(method_path_data),ZOO_EPHEMERAL);
        }
    }

    std::cout<<"RpcProvider start service at ip:"<<ip<<" port:"<<port<<std::endl;

    //启动网络服务
    m_tcpServer.start();
    m_loop.loop();
}

//连接时的回调函数
void RpcProvider::onConnection(const muduo::net::TcpConnectionPtr& conn)
{
    if(!conn->connected())
    {
        conn->shutdown();
    }
}

/*
在框架内部，RpcProvider和RpcConsumer协商好了之间通信用的protobuf数据类型
1.通过”service_name method_name args“来调用远程rpc方法——>定义相应的proto的message类型，进行数据头的序列化和反序列化
2.但每个方法的args个数都不一样，如何定义message类型呢？
    （1）可以不将args定义在proto的message中，而是将args_size定义在message中，之后紧随args
    （2）但这样message与args同时传来，如何反序列化message？——>类比”网络协议格式“
3.综上：header_size(4字节)+header_str+args_str
    <1>header_size：header_str序列化后的大小（因为int类型是4个字节，proto序列化后是二进制，故4字节即可）
    <2>header_str：序列化后的message类型(包含：service_name method_name args_size)
    <3>args_str: 参数
*/
//已建立连接用户的读写事件回调，如果远程有一个rpc服务的调用请求，那么OnMessage方法就会响应
void RpcProvider::onMessage(const muduo::net::TcpConnectionPtr& conn,muduo::net::Buffer* buffer,muduo::Timestamp time)
{
    //获取消息（转为字符串）
    std::string recv_buf=buffer->retrieveAllAsString();

    //用copy——>解析：header_size(4字节)+header_str+args_str
    //1.解析header_size
    uint32_t header_size=0;
    recv_buf.copy((char*)&header_size,4,0);

    //2.解析header_str
    std::string header_str=recv_buf.substr(4,header_size);

    //反序列化header_str,获取service_name method_name args_size
    mprpc::RpcHeader rpcHeader;
    std::string service_name;
    std::string method_name;
    uint32_t args_size;
    if(rpcHeader.ParseFromString(header_str))
    {
        //数据头header_str反序列化成功
        service_name=rpcHeader.service_name();
        method_name=rpcHeader.method_name();
        args_size=rpcHeader.args_size();

    }
    else
    {
        //数据头header_str反序列化失败        
        std::cout<<"parse rpcHeader:"<<header_str<<" error"<<std::endl;
        return;
    }

    //3.解析args_str（args_str中请求方通过proto序列化后的）
    std::string args_str=recv_buf.substr(4+header_size,args_size);

    //打印调试信息
    std::cout<<"================================================="<<std::endl;
    std::cout<<"header_size:"<<header_size<<std::endl;
    std::cout<<"header_str:"<<header_str<<std::endl;
    std::cout<<"service_name:"<<service_name<<std::endl;
    std::cout<<"method_name:"<<method_name<<std::endl;
    std::cout<<"args_size:"<<args_size<<std::endl;
    std::cout<<"args_str"<<args_str<<std::endl;
    std::cout<<"================================================="<<std::endl;    


    //获取service对象和method描述符
    auto it=m_serviceInfoMap.find(service_name);
    if(it==m_serviceInfoMap.end())
    {
        std::cout<<"not find service:"<<service_name<<std::endl;
        return;
    }
    auto mit=it->second.m_methodMap.find(method_name);
    if(mit==it->second.m_methodMap.end())
    {
        std::cout<<"not find service:"<<service_name<<" method:"<<method_name<<std::endl;
    }
    google::protobuf::Service* service=it->second.service; //获取service对象
    const google::protobuf::MethodDescriptor *methodDsc=mit->second; //获取method描述符

    //我们在protobuf中定义的message生成的类，都继承message
    //GetRequestPrototype()：获取某个方法的请求消息类型的原型。GetRequestPrototype 返回一个 google::protobuf::Message 类型的对象，表示这个请求消息的原型。
    //1.获取请求消息类型的原型
    google::protobuf::Message* request=service->GetRequestPrototype(methodDsc).New();
    //2.反序列化
    if(!request->ParseFromString(args_str))
    {
        std::cout<<"request parse error,content:"<<args_str<<std::endl;
    }

    //3.获取响应消息的原型
    google::protobuf::Message* response=service->GetResponsePrototype(methodDsc).New();

    //4.给下面的method方法的调用，绑定一个Closure的回调函数。
        //google::protobuf::Closure 是一个用于异步操作回调的抽象类。
        //通常通过继承 Closure 类并实现 Run() 方法来定义回调逻辑。
        //可以使用 google::protobuf::NewCallback 来简化回调的创建过程。
    //用户通过Closure *done调用done->run()是，会调用绑定的回调函数
    google::protobuf::Closure *done=google::protobuf::NewCallback<RpcProvider,
                                                                const muduo::net::TcpConnectionPtr&,
                                                                google::protobuf::Message*>
                                                                (this,
                                                                &RpcProvider::SendRpcResponse,
                                                                conn,response);//有多个重载版本，选择合适的


    service->CallMethod(methodDsc,nullptr,request,response,done);


}

//Closure的回调操作，用于序列化rpc的响应和网络发送
void RpcProvider::SendRpcResponse(const muduo::net::TcpConnectionPtr& conn,google::protobuf::Message* response)
{
    std::string response_str;
    if(response->SerializeToString(&response_str)) //对response进行序列化
    {
        //序列化成功后，通过网络把rpc方法执行的结果发送给rpc的调用方
        conn->send(response_str);
    }
    else
    {
        std::cout<<"serialize response_str error!"<<std::endl;
    }
    //模拟http的短连接服务，有rpcprovider主动断开链接
    conn->shutdown();
}