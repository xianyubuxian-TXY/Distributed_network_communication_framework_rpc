#pragma once
#include<iostream>
#include<string>
#include "lockqueue.h"


enum LogLevel
{
    INFO, //普通信息
    ERROR, //错误信息
};

//Mprpc框架提供的日志系统(单例模式)
class Logger
{
public:
    //获取日志的单例
    static Logger& GetInstance();
    //设置日志级别
    void SetLogLevel(LogLevel level);
    //写日志
    void Log(std::string msg);
private:
    int m_loglevel; //记录日志级别
    LockQueue<std::string> m_lckQue; //日志缓存队列

    Logger();
    Logger(const Logger&)=delete;
    Logger(Logger&&)=delete;
    Logger operator=(const Logger&)=delete;
    Logger operator=(Logger&&)=delete;

};

//定义宏：给用户输出日志信息
#define LOG_INFO(logmsgformat,...)\
    do  \
    {   \
        Logger &logger=Logger::GetInstance(); \
        logger.SetLogLevel(INFO); \
        char c[1024]={0}; \
        snprintf(c,1024,logmsgformat,##__VA_ARGS__); \
        logger.Log(c);  \
    }while(0);

#define LOG_ERR(logmsgformat,...) \
    do  \
    {   \
        Logger &logger=Logger::GetInstance(); \
        logger.SetLogLevel(ERROR); \
        char c[1024]={0}; \
        snprintf(c,1024,logmsgformat,##__VA_ARGS__); \
        logger.Log(c);  \
    }while(0);