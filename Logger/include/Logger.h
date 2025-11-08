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
#include <utility>
#include "common.h"
#include "LogThreadLocal.h"
#include "LogFileSystem.h"
enum class motifyType
{
    LEVEL,
    OUTPUTMODE,
    LOGFILE,
    MAXFILEBYTE,
    MAXFILENUMBERS,
    KEEPLASTLOGS
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
    Logger(const Logger &) = delete;
    Logger &operator=(const Logger &) = delete;
    template <typename T>
    void log(T &&message, LogLevel level, const char *file, int line, OutPutMode output = OutPutMode::UNKNOWN)
    {
        loggerconfig = std::atomic_load(&loggerConfig);
        if (level < loggerconfig->loglevel)
        {
            return;
        }
        if (output == OutPutMode::UNKNOWN)
        {
            output = loggerconfig->outputmode;
        }
        if (!loggerthreadlocal)
        {
            loggerthreadlocal = std::make_unique<LogThreadLocal>(loggerQueue);
        }
        loggerthreadlocal->appendMessage(std::forward<T>(message), level, output, file, line);
    }
    template <typename... Args>
    void logf(LogLevel level, const char *file, int line,
              const char *fmt, Args&&... args)
    {
        loggerconfig = std::atomic_load(&loggerConfig);
        if (level < loggerconfig->loglevel)
        {
            return;
        }
        OutPutMode output;
        output = loggerconfig->outputmode;
        if (!loggerthreadlocal)
        {
            loggerthreadlocal = std::make_unique<LogThreadLocal>(loggerQueue);
        }
        int size = std::snprintf(nullptr, 0, fmt, args...) + 1; // +1 for '\0'
        std::string message;
        message.resize(size);
        std::snprintf(message.data(), size, fmt, args...);
        message.pop_back(); 
        loggerthreadlocal->appendMessage(std::move(message), level, output, file, line);
    }
    template <typename... Args>
    void logf(LogLevel level,OutPutMode output ,const char *file, int line,
              const char *fmt, Args&&... args)
    {
        loggerconfig = std::atomic_load(&loggerConfig);
        if (level < loggerconfig->loglevel)
        {
            return;
        }
        if (!loggerthreadlocal)
        {
            loggerthreadlocal = std::make_unique<LogThreadLocal>(loggerQueue);
        }
        // 估计需要的缓冲区大小
        int size = std::snprintf(nullptr, 0, fmt, args...) + 1; // +1 for '\0'
        std::string message;
        message.resize(size);
        std::snprintf(message.data(), size, fmt, args...);
        message.pop_back(); // 去掉 '\0'
        loggerthreadlocal->appendMessage(std::move(message), level, output, file, line);
    }
public:
private:
    void backgroundProcess();
    const std::string &geCurrenttime();
    const char *infoString(LogLevel level);

private:
    std::shared_ptr<struct LoggerConfig> loggerconfig;
    static thread_local std::unique_ptr<LogThreadLocal> loggerthreadlocal;
    std::unique_ptr<LogFileSystem> logfilesystem;
    struct LoggerQueue loggerQueue;
    std::unique_ptr<std::thread> backgroundthread;
    std::queue<struct Loggermessage> backgroundqueue;
    std::ofstream logstream;
    std::size_t currentfilebyte = 0;
    size_t cachedSec;
    std::string cachedWallTime;
};
template <typename T>
void motifyConfig(motifyType type, T value)
{
    auto oldconfig = std::atomic_load(&loggerConfig);
    auto newconfig = std::make_shared<struct LoggerConfig>(*oldconfig);
    if constexpr (std::is_same<T, LogLevel>::value)
    {
        if (type == motifyType::LEVEL)
        {
            newconfig->loglevel = value;
        }
    }
    else if constexpr (std::is_same<T, OutPutMode>::value)
    {
        if (type == motifyType::OUTPUTMODE)
        {
            newconfig->outputmode = value;
        }
    }
    else if constexpr (std::is_same<T, std::size_t>::value)
    {
        if (type == motifyType::MAXFILEBYTE)
        {
            newconfig->maxfilebytes = value;
        }
         if (type == motifyType::MAXFILENUMBERS)
        {
            newconfig->maxfilenumbers = value;
        }
    }
    else if constexpr (std::is_same<T, int>::value)
    {
        if (type == motifyType::MAXFILENUMBERS)
        {
            newconfig->maxfilenumbers = value;
        }
    }
    else if constexpr (std::is_same<T, bool>::value)
    {
        if (type == motifyType::KEEPLASTLOGS)
        {
            newconfig->keeplastlogs = value;
        }
    }
    else if constexpr (std::is_same<T, std::string>::value)
    {
        if (type == motifyType::LOGFILE)
        {
            std::string t = std::string(PROJECT_SOURCE_DIR) + std::string("/logs/") + value;
            auto pos = t.rfind(".txt");
            if (pos == std::string::npos || t.size() - pos != 4)
            {
                t += ".txt";
            }
            newconfig->logfile = t;
        }
    }
    std::atomic_store(&loggerConfig, newconfig);
}

void inline LOG_SET_KEEPLASTLOGS(bool keep)
{
    motifyConfig<bool>(motifyType::KEEPLASTLOGS, keep);
}
void inline LOG_SET_LEVEL(LogLevel level)
{
    motifyConfig<LogLevel>(motifyType::LEVEL, level);
}
void inline LOG_SET_FILES(const std::string &filepath)
{
    motifyConfig<std::string>(motifyType::LOGFILE, filepath);
}
void inline LOG_SET_OUTPUTMODE(OutPutMode mode)
{
    motifyConfig<OutPutMode>(motifyType::OUTPUTMODE, mode);
}
void inline LOG_SET_MAXFILEBYTE(std::size_t size)
{
    motifyConfig<std::size_t>(motifyType::MAXFILEBYTE, size);
}
void inline LOG_SET_MAXFILENUM(std::size_t number)
{
    motifyConfig<size_t>(motifyType::MAXFILENUMBERS, number);
}
size_t inline LOG_GET_MAXFILENUM()
{
    auto config = std::atomic_load(&loggerConfig);
    return config->maxfilenumbers;
}
size_t inline LOG_GET_MAXBYTES()
{
    auto config = std::atomic_load(&loggerConfig);
    return config->maxfilebytes;
}
LogLevel inline LOG_GET_LEVEL()
{
    auto config = std::atomic_load(&loggerConfig);
    return config->loglevel;
}
OutPutMode inline LOG_GET_OUTPUTMODE()
{
    auto config = std::atomic_load(&loggerConfig);
    return config->outputmode;
}
bool inline LOG_GET_KEEPLASTLOGS()
{
    auto config = std::atomic_load(&loggerConfig);
    return config->keeplastlogs;
}

#define LOG_DEBUG(message) Logger::Instance().log(message, LogLevel::DEBUG, __FILE__, __LINE__);
#define LOG_INFO(message) Logger::Instance().log(message, LogLevel::INFO, __FILE__, __LINE__);
#define LOG_WARN(message) Logger::Instance().log(message, LogLevel::WARN, __FILE__, __LINE__);
#define LOG_ERROR(message) Logger::Instance().log(message, LogLevel::ERROR, __FILE__, __LINE__);
#define LOG_FDEBUG(message) Logger::Instance().log(message, LogLevel::DEBUG, __FILE__, __LINE__, OutPutMode::FILE);
#define LOG_FINFO(message) Logger::Instance().log(message, LogLevel::INFO, __FILE__, __LINE__, OutPutMode::FILE);
#define LOG_FWARN(message) Logger::Instance().log(message, LogLevel::WARN, __FILE__, __LINE__, OutPutMode::FILE);
#define LOG_FERROR(message) Logger::Instance().log(message, LogLevel::ERROR, __FILE__, __LINE__, OutPutMode::FILE);
#define LOG_CDEBUG(message) Logger::Instance().log(message, LogLevel::DEBUG, __FILE__, __LINE__, OutPutMode::CONSOLE);
#define LOG_CINFO(message) Logger::Instance().log(message, LogLevel::INFO, __FILE__, __LINE__, OutPutMode::CONSOLE);
#define LOG_CWARN(message) Logger::Instance().log(message, LogLevel::WARN, __FILE__, __LINE__, OutPutMode::CONSOLE);
#define LOG_CERROR(message) Logger::Instance().log(message, LogLevel::ERROR, __FILE__, __LINE__, OutPutMode::CONSOLE);
// 格式化宏：带 fmt + 可变参数
#define LOGF_DEBUG(fmt, ...) \
    Logger::Instance().logf(LogLevel::DEBUG, __FILE__, __LINE__, fmt, ##__VA_ARGS__)
#define LOGF_INFO(fmt, ...) \
    Logger::Instance().logf(LogLevel::INFO, __FILE__, __LINE__, fmt, ##__VA_ARGS__)
#define LOGF_WARN(fmt, ...) \
     Logger::Instance().logf(LogLevel::WARN, __FILE__, __LINE__, fmt, ##__VA_ARGS__) 
#define LOGF_ERROR(fmt, ...) \
    Logger::Instance().logf(LogLevel::ERROR, __FILE__, __LINE__, fmt, ##__VA_ARGS__)    
#define LOGF_FDEBUG(fmt, ...) \
    Logger::Instance().logf(LogLevel::DEBUG, OutPutMode::FILE, __FILE__, __LINE__, fmt, ##__VA_ARGS__)
#define LOGF_FINFO(fmt, ...) \
    Logger::Instance().logf(LogLevel::INFO, OutPutMode::FILE, __FILE__, __LINE__, fmt, ##__VA_ARGS__)
#define LOGF_FWARN(fmt, ...) \
    Logger::Instance().logf(LogLevel::WARN, OutPutMode::FILE, __FILE__, __LINE__, fmt, ##__VA_ARGS__)
#define LOGF_FERROR(fmt, ...) \
    Logger::Instance().logf(LogLevel::ERROR, OutPutMode::FILE, __FILE__, __LINE__, fmt, ##__VA_ARGS__)
#define LOGF_CDEBUG(fmt, ...) \
    Logger::Instance().logf(LogLevel::DEBUG, OutPutMode::CONSOLE, __FILE__, __LINE__, fmt, ##__VA_ARGS__)       
#define LOGF_CINFO(fmt, ...) \
    Logger::Instance().logf(LogLevel::INFO, OutPutMode::CONSOLE, __FILE__, __LINE__, fmt, ##__VA_ARGS__)
#define LOGF_CWARN(fmt, ...) \
    Logger::Instance().logf(LogLevel::WARN, OutPutMode::CONSOLE, __FILE__, __LINE__, fmt, ##__VA_ARGS__)
#define LOGF_CERROR(fmt, ...) \
    Logger::Instance().logf(LogLevel::ERROR, OutPutMode::CONSOLE, __FILE__, __LINE__, fmt, ##__VA_ARGS__)
