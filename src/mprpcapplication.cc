#include "mprpcapplication.h"
#include<iostream>
#include <unistd.h> 

MprpcConfig MprpcApplication::m_config= MprpcConfig(); //静态成员变量初始化

void ShowArgsHelp()
{
    std::cout<<"Usage: ./exe -i <config_file>"<<std::endl;
}

// .exe -i config.conf
void MprpcApplication::Init(int argc,char** argv)
{
    if(argc<2)
    {
        ShowArgsHelp();
        exit(EXIT_FAILURE);
    }

    int opt=0;
    std::string config_file;
    //getopt:命令行参数解析
    while((opt=getopt(argc,argv,"i:"))!=-1)
    {
        switch(opt)
        {
            case 'i':
                config_file=optarg;
                break;
            case '?':
                ShowArgsHelp();
                exit(EXIT_FAILURE);
            default:
            break;
        }
    }

    //开始加载配置文件： rpcserverip rpcserverport zookeeperip zookeeperport
    m_config.LoadConfigFile(config_file.c_str());

    std::cout<<"rpcserverip:"<<m_config.Load("rpcserverip")<<std::endl;
    std::cout<<"rpcserverport:"<<m_config.Load("rpcserverport")<<std::endl;
    std::cout<<"zookeeperip:"<<m_config.Load("zookeeperip")<<std::endl;
    std::cout<<"zookeeperport:"<<m_config.Load("zookeeperport")<<std::endl;
}

MprpcApplication& MprpcApplication::GetInstance()
{
    static MprpcApplication app;
    return app;
}

MprpcConfig& MprpcApplication::getConfig()
{
    return m_config;    
}