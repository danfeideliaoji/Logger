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
