#include <string>
#include <chrono>
#include <ctime>
#include <sstream>
#include <iomanip>
#include <fstream>
#include <mutex>
#include <iostream>
#include <thread>

#include "Logger.h"
bool LoggerConfig::ifkeeplastlogs = true;
bool Logger::isexit = false;
thread_local std::unique_ptr<LogThreadLocal> Logger::loggerthreadlocal(nullptr);
Logger::Logger()
{
    initFromConfig();
    Logger::isexit = true;
    logfilesystem = std::make_unique<LogFileSystem>(loggerconfig);
    currentfilebyte = logfilesystem->logsInit(LoggerConfig::ifkeeplastlogs);
    logfilesystem->fileCreate(logstream);
    backgroundthread = std::make_unique<std::thread>(&Logger::backgroundProcess, this);
}
void Logger::initFromConfig()
{
    maxfilebytes = loggerconfig.maxfilebytes;
    logfile = loggerconfig.logfile;
    flushuntervalms = loggerconfig.flushuntervalms;
}
Logger::~Logger()
{
    {
        std::unique_lock<std::mutex> lock(loggerQueue.mutex);
        loggerQueue.stop = true;
        loggerQueue.cv.notify_all();
    }
    backgroundthread->join();
    logstream.close();
}
void Logger::backgroundProcess()
{
    while (true)
    {
        //  std::cerr<<loggerQueue.stop<<"a";
        {
            // std::this_thread::sleep_for(flushuntervalms);
            std::unique_lock<std::mutex> lock(loggerQueue.mutex);
            loggerQueue.cv.wait(lock, [this]
                                { return !loggerQueue.logQueue.empty() || loggerQueue.stop; });
            if (loggerQueue.stop && loggerQueue.logQueue.empty())
            {
                break;
            }
            backgroundqueue.swap(loggerQueue.logQueue);
        }
        while (!backgroundqueue.empty())
        {
            auto t = backgroundqueue.front();
            if (t.output & OutPutMode::CONSOLE)
            {
                std::cout << infoString(t.level) << "[" << geCurrenttime() << "]["
                          << t.threadid << "]["
                          << t.file << ":" << t.line << " " << t.message << "\n";
            }
            if (t.output & OutPutMode::FILE)
            {
                logstream << infoString(t.level) << "[" << geCurrenttime() << "]["
                          << t.threadid << "]["
                          << t.file << ":" << t.line << "]" << t.message << "\n";
            }
            currentfilebyte += t.message.size() + 48;
            if (currentfilebyte >= loggerconfig.maxfilebytes * 0.95)
            {
                logfilesystem->logRotate(logstream);
                currentfilebyte = 0;
            }
            backgroundqueue.pop();
        }
    }
}

void Logger::log(std::string message, LogLevel level, const char *file, int line, OutPutMode output)
{
    if (!loggerthreadlocal)
    {
        loggerthreadlocal = std::make_unique<LogThreadLocal>(loggerconfig, loggerQueue);
    }
    loggerthreadlocal->appendMessage(message, level, output, file, line);
}
const std::string &Logger::geCurrenttime()
{
    using namespace std::chrono;
    auto now = system_clock::now();
    time_t sec = system_clock::to_time_t(time_point_cast<seconds>(now));
    std::tm tm_time;
    if (sec != cachedSec)
    {
        cachedSec = sec;
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
const std::string Logger::infoString(LogLevel level)
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