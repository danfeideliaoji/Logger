#pragma once
#include <string>
#include <thread>
#include<sstream>
#include"common.h"
class LogThreadLocal
{
public:
    LogThreadLocal(struct LoggerConfig& loggerConfig,struct LoggerQueue& Loggerqueue);
    ~LogThreadLocal();
    void appendMessage( std::string &message,
                       LogLevel level, OutPutMode output,const char *file, int line); 
private:
    const std::string infoString(LogLevel level);
    void messagePush();
    void initFromConfig();
private:
    struct LoggerConfig& loggerconfig;    
    struct LoggerQueue &loggerQueue;
    std::queue<struct Loggermessage> loggermessages;
    LogLevel loglevel;
    OutPutMode outputmode;
    size_t batchsize ;             
    std::chrono::milliseconds flushuntervalms; 
};
