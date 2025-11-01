#pragma once
#include <string>
#include <fstream>
#include <mutex>
#include <queue>
#include <memory>
#include<thread>
#include<iostream>
#include<condition_variable>
#include<sstream>
#include<chrono>
enum class LogLevel
{
    DEBUG,
    INFO,
    WARN,
    ERROR
};
enum class OutPutMode
{
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
struct LoggerQueue{
    std::queue<std::string> logQueue;
    std::mutex mutex;
    std::condition_variable cv;
    bool stop=false;
};
struct LoggerConfig {
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
class Logger
{
public:
    Logger();
    ~Logger();
    static Logger &Instance()
    {
        static Logger instance;
        return instance;
    }
    void log(const std::string &message, LogLevel level, const char *file, int line);

public:
    struct LoggerConfig loggerconfig;
    static bool isexit;
private:
    void logsJudge();
    std::string getCurrentTime();
    std::string getCurrentId();
    void fileCreate();
    void ossPush();
    void messageCreate(const std::string &message, const std::string &currenttime,
                 LogLevel level, const char *file, int line);
    void notify();
    void logRotate();
    void logUpdateDelete();            
    void backgroundProcess();
private:
    struct LoggerQueue loggerQueue;
    std::unique_ptr<std::thread> backgroundthread;
    std::queue<std::string> backgroundqueue;
    std::ofstream logstream;
    std::size_t currentfilebyte = 0;
    std::size_t currentbuffer=0;
    std::size_t filenumber = 1;
    std::string currenttime;
    std::ostringstream oss;
    std::ostringstream idoss;
    std::ostringstream timeoss;
    std::chrono::time_point<std::chrono::system_clock> lasttimeupdate;
    std::chrono::time_point<std::chrono::system_clock> lastnotifytime=std::chrono::system_clock::now();
};
void inline LOG_SET_KEEPLASTLOGS(bool keep){
     LoggerConfig::ifkeeplastlogs= keep;
}
void inline LOG_SET_LEVEL(LogLevel level)
{
    Logger::Instance().loggerconfig.loglevel = level;
}
void inline LOG_SET_FILES(const std::string &filepath)
{
    Logger::Instance().loggerconfig.logfile = filepath;
}
void inline LOG_SET_OUTPUTMODE(OutPutMode mode)
{
    Logger::Instance().loggerconfig.outputmode = mode;
}
void inline LOG_SET_MAXFILEBYTE(std::size_t size)
{
    Logger::Instance().loggerconfig.maxfilebytes = size;
}
void inline LOG_SET_MAXFILENUMBER(std::size_t number)
{
    Logger::Instance().loggerconfig.maxfilenumbers = number;
}
size_t inline LOG_GET_FILES()
{
    return Logger::Instance().loggerconfig.maxfilenumbers;
}
size_t inline LOG_GET_MAXBYTES()
{
    return Logger::Instance().loggerconfig.maxfilebytes;
}
#define LOG_DEBUG(message) Logger::Instance().log(message, LogLevel::DEBUG, __FILE__, __LINE__);
#define LOG_INFO(message) Logger::Instance().log(message, LogLevel::INFO, __FILE__, __LINE__);
#define LOG_WARN(message) Logger::Instance().log(message, LogLevel::WARN, __FILE__, __LINE__);
#define LOG_ERROR(message) Logger::Instance().log(message, LogLevel::ERROR, __FILE__, __LINE__);
