#pragma once
#include "mprpcconfig.h"

//mprpc框架的基础类（单例模式）
class MprpcApplication
{

public:
    static void Init(int argc,char** argv);
    static MprpcApplication& GetInstance();
    static MprpcConfig& getConfig();
private:
    static MprpcConfig m_config; //配置文件对象

    MprpcApplication(){}
    MprpcApplication(const MprpcApplication&)=delete;
    MprpcApplication(MprpcApplication&&)=delete;
    MprpcApplication& operator=(const MprpcApplication&)=delete;
    MprpcApplication& operator=(MprpcApplication&&)=delete;
};