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
{   initFromConfig();
    Logger::isexit = true;
    logfilesystem = std::make_unique<LogFileSystem>(loggerconfig);
    currentfilebyte = logfilesystem->logsInit(LoggerConfig::ifkeeplastlogs);
    logfilesystem->fileCreate(logstream);
    backgroundthread = std::make_unique<std::thread>(&Logger::backgroundProcess, this);
}
void Logger::initFromConfig(){
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
        std::string message;
        while (!backgroundqueue.empty())
        {
            message.append(std::move(backgroundqueue.front()));
            backgroundqueue.pop();
        }
        currentfilebyte += message.size();
        if (currentfilebyte >= loggerconfig.maxfilebytes)
        {
            logfilesystem->logRotate(logstream);
            currentfilebyte = message.size();
            if (currentfilebyte > loggerconfig.maxfilebytes)
            {
                LOG_SET_MAXFILEBYTE(currentfilebyte * 1.1);
                std::cerr << "捕获异常: " << "日志最大字节太小，现调高字节上限: 现最大字节数" + std::to_string(LOG_GET_MAXBYTES()) + "bytes" << std::endl;
            }
        }
        logstream << message;
        message.clear();
    }
}

void Logger::log(const std::string &message, LogLevel level, const char *file, int line, OutPutMode output)
{
    if(!loggerthreadlocal)
    {
        loggerthreadlocal =std::make_unique<LogThreadLocal>(loggerconfig,loggerQueue);
    }
    loggerthreadlocal->appendMesssage(message, level, output, file, line);
}

