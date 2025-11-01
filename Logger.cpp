#include <string>
#include <chrono>
#include <ctime>
#include <sstream>
#include <iomanip>
#include <fstream>
#include <mutex>
#include <iostream>
#include <thread>
#include <filesystem>
#include "Logger.h"
bool LoggerConfig::ifkeeplastlogs = true;
bool Logger::isexit = false;
thread_local std::unique_ptr<LogThreadLocal> Logger::loggerthreadlocal(nullptr);
Logger::Logger()
{   initFromConfig();
    Logger::isexit = true;
    logsJudge();
    fileCreate();
    backgroundthread = std::make_unique<std::thread>(&Logger::backgroundProcess, this);
}
void Logger::initFromConfig(){
    std::unique_lock<std::mutex> lock(loggerconfig.configmutex);
    maxfilebytes = loggerconfig.maxfilebytes;
    maxfilenumbers = loggerconfig.maxfilenumbers;
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
            logRotate();
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
void Logger::logsJudge()
{
    using namespace std::filesystem;
    if (LoggerConfig::ifkeeplastlogs)
    {
        if (!exists("./logs"))
        {
            create_directory("./logs");
        }
        else
        {
            currentfilebyte = file_size(loggerconfig.logfile);
            filenumber = 0;
            for (auto &entry : directory_iterator("logs"))
            {
                if (is_regular_file(entry.path()))
                {
                    filenumber++;
                }
            }
        }
    }
    else
    {
        if (exists("./logs"))
        {
            try
            {
                remove_all("./logs");
            }
            catch (const filesystem_error &e)
            {
                std::cerr << "删除失败: " << e.what() << std::endl;
            }
            create_directory("./logs");
        }
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
void Logger::fileCreate()
{
    if (!(loggerconfig.outputmode & OutPutMode::FILE))
    {
        return;
    }
    logstream.open(loggerconfig.logfile, std::ios::app);
    if (!logstream.is_open())
    {
        logstream.open(loggerconfig.logfile, std::ios::out);
    }
}


void Logger::logRotate()
{
    logstream.close();
    filenumber++;
    logUpdateDelete();
    fileCreate();
}
void Logger::logUpdateDelete()
{
    using namespace std::filesystem;
    while (filenumber >= loggerconfig.maxfilenumbers)
    {
        remove(loggerconfig.logfile + "." + std::to_string(filenumber - 1));
        filenumber--;
    }
    for (int i = filenumber - 1; i >= 0; --i)
    {
        std::string oldname;
        if (i == 0)
        {
            oldname = loggerconfig.logfile;
        }
        else
            oldname = loggerconfig.logfile + "." + std::to_string(i);
        std::string newname = loggerconfig.logfile + "." + std::to_string(i + 1);
        if (exists(oldname))
        {
            rename(oldname, newname);
        }
    }
}