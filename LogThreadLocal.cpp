#include "LogThreadLocal.h"
#include <chrono>
#include <mutex>
#include <iostream>
LogThreadLocal::LogThreadLocal(struct LoggerConfig& loggerConfig,struct LoggerQueue& Loggerqueue )
    : loggerQueue(Loggerqueue),loggerconfig(loggerConfig)
{   
    initFromConfig();
}
void LogThreadLocal::initFromConfig()
{
    std::unique_lock<std::mutex> lock(loggerconfig.configmutex);
    loglevel = loggerconfig.loglevel;
    outputmode = loggerconfig.outputmode;
    batchsize = loggerconfig.batchsize;
    flushuntervalms = loggerconfig.flushuntervalms;
}
LogThreadLocal::~LogThreadLocal()
{
     messagePush();
}
void LogThreadLocal::appendMessage( std::string &message,
                                    LogLevel level, OutPutMode output, const char *file, int line)
{
    if (level < loglevel)
    {
        return;
    }
    if (output == OutPutMode::UNKNOWN)
    {
        output = outputmode;
    }
    loggermessages.push({std::move(message),level,output,file,line,std::this_thread::get_id()});
    if(loggermessages.size()>=batchsize){
        messagePush();
    }
}
void LogThreadLocal::messagePush()
{   std::unique_lock<std::mutex> lock(loggerQueue.mutex);
    while(!loggermessages.empty())
    {
      loggerQueue.logQueue.push(std::move(loggermessages.front()));
      loggermessages.pop();
    }
    loggerQueue.cv.notify_one();
}
