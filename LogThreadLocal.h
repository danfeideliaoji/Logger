#pragma once
#include <string>
#include <thread>
#include<chrono>
#include<sstream>
#include"common.h"
class LogThreadLocal
{
public:
    LogThreadLocal(struct LoggerQueue& Loggerqueue);
    ~LogThreadLocal();
    void appendMessage( std::string &message,
                       LogLevel level, OutPutMode output,const char *file, int line); 
private:
    const std::string infoString(LogLevel level);
    void messagePush();
private:
    std::chrono::steady_clock::time_point lastflushTime;
    std::thread::id mythreadid;
    std::shared_ptr<struct LoggerConfig>loggerconfig;    
    struct LoggerQueue &loggerQueue;
    std::queue<struct Loggermessage> loggermessages;
};
