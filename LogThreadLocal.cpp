#include "LogThreadLocal.h"
#include <chrono>
#include <mutex>
#include <iostream>
LogThreadLocal::LogThreadLocal(struct LoggerQueue& Loggerqueue )
    : loggerQueue(Loggerqueue)
{   
     loggerconfig=std::atomic_load(&loggerConfig);
     mythreadid=std::this_thread::get_id();
}
LogThreadLocal::~LogThreadLocal()
{
     messagePush();
}
void LogThreadLocal::appendMessage( std::string &message,
                                    LogLevel level, OutPutMode output, const char *file, int line)
{
    loggerconfig=std::atomic_load(&loggerConfig);
    if (level < loggerconfig->loglevel)
    {
        return;
    }
    if (output == OutPutMode::UNKNOWN)
    {
        output = loggerconfig->outputmode;
    }
    loggermessages.push({std::move(message),level,output,file,line,mythreadid});
    auto now=std::chrono::steady_clock::now();
    if( loggermessages.size()>=loggerconfig->batchsize||std::chrono::duration_cast<std::chrono::milliseconds>(now-lastflushTime)>=loggerconfig->flushuntervalms
   )
    {
        lastflushTime=now;
        messagePush();
        return;
    }
    
}
void LogThreadLocal::messagePush()
{   
    std::unique_lock<std::mutex> lock(loggerQueue.mutex);
    while(!loggermessages.empty())
    {
      loggerQueue.logQueue.push(std::move(loggermessages.front()));
      loggermessages.pop();
    }
    loggerQueue.cv.notify_one();
}
