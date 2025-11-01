#pragma once
#include<condition_variable>
#include <mutex>
#include <string>
#include <queue>
#include<thread>
enum class LogLevel
{   
    DEBUG,
    INFO,
    WARN,
    ERROR
};
enum class OutPutMode
{   UNKNOWN=0,
    CONSOLE = 1,
    FILE = 2,
};
inline OutPutMode operator|(OutPutMode a, OutPutMode b)
{
    return static_cast<OutPutMode>(static_cast<int>(a) | static_cast<int>(b));
}
inline int operator&(OutPutMode a, OutPutMode b)
{
    return (static_cast<int>(a) & static_cast<int>(b));
}
struct LoggerConfig {
    std::mutex configmutex;
    size_t maxfilebytes = 1 * 1024 * 1024;
    int maxfilenumbers = 10;
    LogLevel loglevel = LogLevel::DEBUG;
    OutPutMode outputmode = OutPutMode::FILE;
    std::string logfile = "./logs/log.txt";
    size_t batchsize = 10;          // 一次写 10 条日志
    std::chrono::milliseconds flushuntervalms{100}; // 等待 50ms 即使队列未满也写
    std::size_t bufferlimit=1024;
    static bool ifkeeplastlogs;
};
struct Loggermessage{
    std::string message;
    LogLevel level;
    OutPutMode output;
    const char* file;
    int line;
    std::thread::id threadid;
}; 
struct LoggerQueue{
    std::queue<struct Loggermessage> logQueue;
    std::mutex mutex;
    std::condition_variable cv;
    bool stop=false;
};