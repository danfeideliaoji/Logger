#pragma once
#include <string>
#include <thread>
#include <chrono>
#include<utility>
#include <sstream>
#include "common.h"
class LogThreadLocal
{
public:
    LogThreadLocal(struct LoggerQueue &Loggerqueue);
    ~LogThreadLocal();
    template<typename T>
    void appendMessage(T&& message,
                       LogLevel level, OutPutMode output, const char *file, int line)
    {      
        loggermessages.push({std::forward<T>(message), level, output, file, line, mythreadid});
        auto now = std::chrono::steady_clock::now();
        if (loggermessages.size() >= loggerconfig->batchsize || 
        std::chrono::duration_cast<std::chrono::milliseconds>(now - lastflushTime) >= loggerconfig->flushuntervalms)
        {
            loggerconfig = std::atomic_load(&loggerConfig);
            lastflushTime = now;
            messagePush();
            return;
        }
    }

private:
    const std::string infoString(LogLevel level);
    void messagePush();

private:
    std::chrono::steady_clock::time_point lastflushTime;
    std::thread::id mythreadid;
    std::shared_ptr<struct LoggerConfig> loggerconfig;
    struct LoggerQueue &loggerQueue;
    std::queue<struct Loggermessage> loggermessages;
};
