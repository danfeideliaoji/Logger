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
    void appendMesssage(const std::string &message,
                       LogLevel level, OutPutMode output,const char *file, int line); 
private:
    const std::string infoString(LogLevel level);
    const std::string& geCurrenttime();
    const std::string&  getThreadId();
    void bufferPush();
    char* lineToString(time_t line);
    void initFromConfig();
private:
    struct LoggerConfig& loggerconfig;    
    struct LoggerQueue &loggerQueue;
    std::string cachedWallTime;
    time_t cachedSec;
    std::string temp;
    std::string buffer;
    std::string idstring;
    char linebuf[20];
    LogLevel loglevel;
    OutPutMode outputmode;
    size_t bufferlimit;
    size_t batchsize ;             
    std::chrono::milliseconds flushuntervalms; 
};
