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
std::shared_ptr<struct LoggerConfig> loggerConfig(std::make_shared<struct LoggerConfig>());
thread_local std::unique_ptr<LogThreadLocal> Logger::loggerthreadlocal(nullptr);
Logger::Logger()
{   loggerconfig=std::atomic_load(&loggerConfig);
    logfilesystem = std::make_unique<LogFileSystem>();
    currentfilebyte = logfilesystem->logsInit();
    logfilesystem->fileCreate(logstream);
    backgroundthread = std::make_unique<std::thread>(&Logger::backgroundProcess, this);
}
Logger::~Logger()
{
    {   
        std::unique_lock<std::mutex> lock(loggerQueue.mutex);
        loggerQueue.stop = true;
        loggerQueue.cv.notify_one();
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
        loggerconfig=std::atomic_load(&loggerConfig);
        const std::string& timeStr = geCurrenttime();  // 批量处理时使用同一个时间字符串
        while (!backgroundqueue.empty()){
            auto t = backgroundqueue.front();
            if (t.output & OutPutMode::CONSOLE)
            {
                std::cout << infoString(t.level) << "[" << timeStr << "]["
                          << t.threadid << "]["
                          << t.file << ":" << t.line << " " << t.message << "\n";
            }
            if (t.output & OutPutMode::FILE)
            {
                logstream << infoString(t.level) << "[" << timeStr << "]["
                          << t.threadid << "]["
                          << t.file << ":" << t.line << "]" << t.message << "\n";
            }
            currentfilebyte += t.message.size() + 48;
            if (currentfilebyte >= loggerconfig->maxfilebytes * 0.95)
            {
                logfilesystem->logRotate(logstream);
                currentfilebyte = 0;
            }
            backgroundqueue.pop();
        }
        if (logstream.is_open()) {
            logstream.flush();
        }
    }   
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
const char* Logger::infoString(LogLevel level)
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