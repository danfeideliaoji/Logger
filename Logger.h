#pragma once
#include <string>
#include <fstream>
#include <mutex>
#include <queue>
#include <memory>
#include <thread>
#include <iostream>
#include <condition_variable>
#include <sstream>
#include <chrono>
#include"common.h"
#include"LogThreadLocal.h"
#include"LogFileSystem.h"
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
    void log(const std::string &message, LogLevel level, const char *file, int line, OutPutMode output = OutPutMode::UNKNOWN);

public:
    struct LoggerConfig loggerconfig;
    static bool isexit;
private:
    void backgroundProcess();
    void initFromConfig();
private:
    static thread_local std::unique_ptr<LogThreadLocal> loggerthreadlocal;
    std::unique_ptr<LogFileSystem> logfilesystem; 
    struct LoggerQueue loggerQueue;
    std::unique_ptr<std::thread> backgroundthread;
    std::queue<std::string> backgroundqueue;
    std::ofstream logstream;
    std::size_t currentfilebyte = 0;
    size_t maxfilebytes ;
    std::string logfile ;
    std::chrono::milliseconds flushuntervalms; // 等待 50ms 即使队列未满也写
};
void inline LOG_SET_KEEPLASTLOGS(bool keep)
{
    LoggerConfig::ifkeeplastlogs = keep;
}
void inline LOG_SET_LEVEL(LogLevel level)
{   std::unique_lock<std::mutex> lock(Logger::Instance().loggerconfig.configmutex);
    Logger::Instance().loggerconfig.loglevel = level;
}
void inline LOG_SET_FILES(const std::string &filepath)
{   std::unique_lock<std::mutex> lock(Logger::Instance().loggerconfig.configmutex);
    Logger::Instance().loggerconfig.logfile = filepath;
}
void inline LOG_SET_OUTPUTMODE(OutPutMode mode)
{   std::unique_lock<std::mutex> lock(Logger::Instance().loggerconfig.configmutex);
    Logger::Instance().loggerconfig.outputmode = mode;
}
void inline LOG_SET_MAXFILEBYTE(std::size_t size)
{   std::unique_lock<std::mutex> lock(Logger::Instance().loggerconfig.configmutex);
    Logger::Instance().loggerconfig.maxfilebytes = size;
}
void inline LOG_SET_MAXFILENUMBER(std::size_t number)
{   std::unique_lock<std::mutex> lock(Logger::Instance().loggerconfig.configmutex);
    Logger::Instance().loggerconfig.maxfilenumbers = number;
}
size_t inline LOG_GET_FILES()
{   std::unique_lock<std::mutex> lock(Logger::Instance().loggerconfig.configmutex);
    return Logger::Instance().loggerconfig.maxfilenumbers;
}
size_t inline LOG_GET_MAXBYTES()
{   std::unique_lock<std::mutex> lock(Logger::Instance().loggerconfig.configmutex);
    return Logger::Instance().loggerconfig.maxfilebytes;
}
#define LOG_DEBUG(message) Logger::Instance().log(message, LogLevel::DEBUG, __FILE__, __LINE__);
#define LOG_INFO(message)  Logger::Instance().log(message, LogLevel::INFO, __FILE__, __LINE__);
#define LOG_WARN(message)  Logger::Instance().log(message, LogLevel::WARN, __FILE__, __LINE__);
#define LOG_ERROR(message) Logger::Instance().log(message, LogLevel::ERROR, __FILE__, __LINE__);
#define LOG_FDEBUG(message)Logger::Instance().log(message, LogLevel::DEBUG, __FILE__, __LINE__, OutPutMode::FILE);
#define LOG_FINFO(message) Logger::Instance().log(message, LogLevel::INFO, __FILE__, __LINE__,  OutPutMode::FILE);
#define LOG_FWARN(message) Logger::Instance().log(message, LogLevel::WARN, __FILE__, __LINE__,  OutPutMode::FILE);
#define LOG_FERROR(message)Logger::Instance().log(message, LogLevel::ERROR, __FILE__, __LINE__, OutPutMode::FILE);
#define LOG_CDEBUG(message)Logger::Instance().log(message, LogLevel::DEBUG, __FILE__, __LINE__,  OutPutMode::CONSOLE);
#define LOG_CINFO(message) Logger::Instance().log(message, LogLevel::INFO, __FILE__, __LINE__,  OutPutMode::CONSOLE);
#define LOG_CWARN(message) Logger::Instance().log(message, LogLevel::WARN, __FILE__, __LINE__,  OutPutMode::CONSOLE);
#define LOG_CERROR(message)Logger::Instance().log(message, LogLevel::ERROR, __FILE__, __LINE__, OutPutMode::CONSOLE);