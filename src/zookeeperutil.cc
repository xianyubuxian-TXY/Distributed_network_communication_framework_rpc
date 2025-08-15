#include "zookeeperutil.h"
#include "mprpcapplication.h"
#include <semaphore.h>
#include <iostream>


//全局的watcher观察器   zkserver该zkclient的通知
void global_watcher(zhandle_t *zh,int type,int state,const char *path,void *watcherCtx)
{
    if(type==ZOO_SESSION_EVENT) //回调的消息类型：与会话相关的消息类型
    {
        if(state==ZOO_CONNECTED_STATE)  //zkclient与zkserver连接成功
        {
            sem_t *sem=(sem_t*)zoo_get_context(zh);
            sem_post(sem);
        }
    }
}

ZkClient::ZkClient():m_zhandle(nullptr){}

ZkClient::~ZkClient()
{
    if(m_zhandle!=nullptr)
    {
        //关闭句柄，释放资源
        zookeeper_close(m_zhandle);
    }
}

// zkclient启动连接zkserver
void ZkClient::Start()
{
    std::string host=MprpcApplication::GetInstance().getConfig().Load("zookeeperip");
    std::string port=MprpcApplication::GetInstance().getConfig().Load("zookeeperport");
    std::string connstr=host+":"+port;

    /*
    zookeeper_mt:多线程版本
    zookeeper的API客户端程序提供了三个线程：
        1.API调用线程（执行zookeeper_init的当前线程）
        2.网络I/O线程 (专门创建一个线程发送zookeeper的I/O连接，连接zkserver，poll)
        3.watcher回调线程
    */
    //参数1：ip：port   参数2：回调函数   参数3：会话超时时间  参数4、5：没用   参数6:0
    //zookeeper_init成功，只代表创建句柄成功，不代表我们(zkclient)与zkserver连接成功
    m_zhandle=zookeeper_init(connstr.c_str(),global_watcher,30000,nullptr,nullptr,0);
    if(nullptr==m_zhandle)
    {
        std::cout<<"zookeeper_init error!"<<std::endl;
    }
    
    //zookeeper_init所在线程与zookeeper连接线程属于不同线程，所以使用“信号量机制”来同步
    sem_t sem;
    sem_init(&sem,0,0);
    //为“zk句柄”绑定一个信号量
    zoo_set_context(m_zhandle,&sem);

    sem_wait(&sem);
    std::cout<<"zookeeper_init success!"<<std::endl;
}

// 在zkserver上根据指定的path创建znode节点
void ZkClient::Create(const char *path,const char *data,int datalen,int state)
{
    char path_buffer[128];
    int bufferlen=sizeof(path_buffer);
    int flag;
    //先判断path表示的节点是否存在，如果存在，就不再重复创建了
    flag=zoo_exists(m_zhandle,path,0,nullptr);
    if(ZNONODE==flag) //表示path表示的节点不存在
    {
        //创建指定path的znode节点
        flag=zoo_create(m_zhandle,path,data,datalen,&ZOO_OPEN_ACL_UNSAFE,state,path_buffer,bufferlen);
        if(ZOK==flag)
        {
            std::cout<<"znode create success... path:"<<path<<std::endl;
        }
        else
        {
            std::cout<<"flag:"<<flag<<std::endl;
            std::cout<<"znode create error... path:"<<path<<std::endl;
            exit(EXIT_FAILURE);
        }
    }
}

//根据参数指定的znode节点路径，获取znode节点的值
std::string ZkClient::GetData(const char *path)
{
    char buffer[64];
    int bufferlen=sizeof(buffer);
    int flag=zoo_get(m_zhandle,path,0,buffer,&bufferlen,nullptr);
    if(ZOK!=flag)
    {
        std::cout<<"get znode error... path:"<<path<<std::endl;
        return "";
    }
    else
    {
        return buffer;
    }
}