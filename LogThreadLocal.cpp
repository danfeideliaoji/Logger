#include "LogThreadLocal.h"
#include <chrono>
#include <mutex>
#include <iostream>
LogThreadLocal::LogThreadLocal(struct LoggerConfig& loggerConfig,struct LoggerQueue& Loggerqueue )
    : loggerQueue(Loggerqueue),loggerconfig(loggerConfig)
{   initFromConfig();
    cachedSec = 0;
    cachedWallTime = "";
    buffer.reserve(bufferlimit);
    std::ostringstream ss;
    ss << std::this_thread::get_id();
    idstring = ss.str();
}
void LogThreadLocal::initFromConfig()
{
    std::unique_lock<std::mutex> lock(loggerconfig.configmutex);
    loglevel = loggerconfig.loglevel;
    outputmode = loggerconfig.outputmode;
    bufferlimit=loggerconfig.bufferlimit;
    batchsize = loggerconfig.batchsize;
    flushuntervalms = loggerconfig.flushuntervalms;
}
LogThreadLocal::~LogThreadLocal()
{
     bufferPush();
}
void LogThreadLocal::appendMesssage(const std::string &message,
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
    temp.append(infoString(level)).append("[").append(geCurrenttime())
    .append("][").append(getThreadId()).append("][").append(file).append(":")
    .append(lineToString(line)).append("]log:").append(message).append("\n");
    if (output & OutPutMode::CONSOLE)
    {
        std::cout << temp;
    }
    if (output & OutPutMode::FILE)
    {
        if (temp.size() + buffer.size() >= bufferlimit)
        {
            bufferPush();
        }
        buffer.append(std::move(temp));
    }
    temp.clear();
    temp.reserve(bufferlimit);
}
void LogThreadLocal::bufferPush()
{
    if (!buffer.empty())
    {
        std::unique_lock<std::mutex> lock(loggerQueue.mutex);
        loggerQueue.logQueue.push(std::move(buffer));
        if(loggerQueue.logQueue.size()>=batchsize)
        {
            loggerQueue.cv.notify_one();
        }
    }
    buffer.clear();  
    buffer.reserve(loggerconfig.bufferlimit);
}
const std::string LogThreadLocal::infoString(LogLevel level)
{
    switch (level)
    {
    case LogLevel::DEBUG:
        return "DEBUG:";
    case LogLevel::INFO:
        return "INFO:";
    case LogLevel::WARN:
        return "WARN:";
    case LogLevel::ERROR:
        return "ERROR:";
    default:
        return "UNKNOWN:";
    }
}
const std::string& LogThreadLocal::geCurrenttime()
{
    using namespace std::chrono;
    auto now = system_clock::now();
    time_t sec = system_clock::to_time_t(time_point_cast<seconds>(now));
    std::tm tm_time;
    if (sec != cachedSec)
    {   cachedSec=sec;
#if defined(_WIN32) || defined(_WIN64)
        localtime_s(&tm_time, &sec);
#else
        localtime_r(&sec, &tm_time);
#endif
        char buf[20];
        std::strftime(buf, sizeof(buf), "%F %T", &tm_time); // "2025-10-28 17:23:14"
        cachedWallTime.assign(buf);                         // 更新缓存
    }
    return cachedWallTime;
}
const std::string& LogThreadLocal::getThreadId()
{
    return idstring;
}
char* LogThreadLocal::lineToString(time_t line)
{
    char* p = linebuf + 20;  // 指向末尾
    *--p = '\0';
    do {
        *--p = '0' + line % 10;
       line /= 10;
    } while (line);
    return p;
}