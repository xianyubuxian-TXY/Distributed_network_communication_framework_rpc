#pragma once
#include<unordered_map>
#include<string>

//rpcsserverip rpcsserverport zookeeperip zookeeperport
//框架读取配置文件类
class MprpcConfig
{
public:
    //负责解析加载配置文件
    void LoadConfigFile(const char* config_file);
    //查询配置项信息
    std::string Load(const std::string& key);
private:
    //去掉字符串前后的空格
    void strip(std::string &str);
    //从字符串中获取key-value
    std::pair<std::string,std::string> get_key_value(const std::string& str);

    std::unordered_map<std::string,std::string> m_configMap;
};