#include "mprpcconfig.h"
#include<iostream>
#include<fstream>
#include<string>

//去掉首尾所有空格
void MprpcConfig::strip(std::string &str)
{
    int idx=str.find_first_not_of(' '); //首部第一个非空格下标
    //首部有空格
    if(idx!=-1)
    {
        str=str.substr(idx); 
    }

    //尾部有空格
    idx=str.find_last_not_of(' '); //尾部第一个非空格下标
    if(idx!=-1)
    {
        str=str.substr(0,idx+1);
    }
}

//通过‘=’获取键值对
std::pair<std::string,std::string> MprpcConfig::get_key_value(const std::string& str)
{
    int idx=str.find('=');
    if(idx==-1)
    {
        std::string err="format of config is invalid";
        throw err;
    }
    std::string key=str.substr(0,idx);
    std::string value=str.substr(idx+1);

    if(key.empty() || value.empty())
    {
        std::string err="format of config is invalid";
        throw err;
    }
    //去除key、value与‘=’之间的空格
    strip(key);
    strip(value);
    return std::make_pair(key,value);
}

//负责解析加载配置文件
void MprpcConfig::LoadConfigFile(const char* config_file)
{
    FILE* fp=fopen(config_file,"r");
    if(fp==nullptr)
    {
        std::cerr<<"mprpcConfig load config file failed, file="<<config_file<<std::endl;
        exit(EXIT_FAILURE);
    }

    while(!feof(fp))
    {
        char buffer[512]={0};
        fgets(buffer,512,fp); //读取一行

        std::string line(buffer); //生成对于的string类型，方便字符串处理
        //去掉末尾的'\n'
        int idx=line.find('\n');
        if(idx!=-1)
        {
            line.erase(idx);
        }
        
        //调用strip去除首尾所有空格
        strip(line);

        //去掉注释行 或 全是空格的行
        if(line[0]=='#' || line.empty())
        {
            continue;
        }

        //解析配置项
        try{
            m_configMap.insert(get_key_value(line));
        }catch (const std::string& err){
            std::cerr<<err<<std::endl;
            continue;
        }
    }
}


//查询配置项信息
std::string MprpcConfig::Load(const std::string& key)
{
    auto it=m_configMap.find(key);
    if(it==m_configMap.end())
    {
        return "";
    }
    else return it->second;
}