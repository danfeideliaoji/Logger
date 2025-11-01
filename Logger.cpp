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
bool Logger::isexit=false;
Logger::Logger()
{   Logger::isexit=true;
    logsJudge();
    fileCreate();
    backgroundthread = std::make_unique<std::thread>(&Logger::backgroundProcess, this);
}
Logger::~Logger()
{
    {
        std::unique_lock<std::mutex> lock(loggerQueue.mutex);
        ossPush();
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
        std::unique_lock<std::mutex> lock(loggerQueue.mutex);
        loggerQueue.cv.wait(lock, [this]
                            { return !loggerQueue.logQueue.empty() || loggerQueue.stop; });
        if (loggerQueue.stop && loggerQueue.logQueue.empty())
        {
            break;
        }
        std::string message;
        std::size_t cnt = 0;
        while (!loggerQueue.logQueue.empty() && cnt <= loggerconfig.batchsize)
        {
            message.append(std::move(loggerQueue.logQueue.front()));
            loggerQueue.logQueue.pop();
            cnt++;
        }
        lock.unlock();
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

void Logger::log(const std::string &message, LogLevel level, const char *file, int line)
{
    std::unique_lock<std::mutex> lock(loggerQueue.mutex);
    if (level < loggerconfig.loglevel)
    {
        return;
    }
    auto now = std::chrono::system_clock::now();
    if (now - lasttimeupdate >= std::chrono::seconds(1))
    {
        lasttimeupdate = now;
        currenttime = getCurrentTime();
    }
    messageCreate(message, currenttime, level, file, line);
    notify();
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
void Logger::ossPush()
{
    if (!oss.str().empty())
    {
        loggerQueue.logQueue.push(std::move(oss.str()));
        oss.str("");
    }
}
void Logger::messageCreate(const std::string &message, const std::string &currenttime,
                           LogLevel level, const char *file, int line)
{
    std::string logstring;
    switch (level)
    {
    case LogLevel::DEBUG:
        logstring = "DEBUG";
        break;
    case LogLevel::INFO:
        logstring = "INFO";
        break;
    case LogLevel::WARN:
        logstring = "WARN";
        break;
    case LogLevel::ERROR:
        logstring = "ERROR";
        break;
    }
    if (currentbuffer + message.size() + 48 > loggerconfig.bufferlimit)
    {
        ossPush();
        currentbuffer = 0;
    }
    oss << "[" << currenttime << "]" << "[" << getCurrentId() << "]"
        << "[" << file << ":" << std::to_string(line) << "]" << "log:" << message << "\n";
    currentbuffer += message.size() + 48;
}
void Logger::notify()
{
    if (loggerQueue.logQueue.size() >= loggerconfig.batchsize ||
        std::chrono::system_clock::now() - lastnotifytime >= loggerconfig.flushuntervalms)
    {
        lastnotifytime = std::chrono::system_clock::now();
        loggerQueue.cv.notify_one();
    }
}
std::string Logger::getCurrentId()
{
    idoss.str("");
    idoss << std::this_thread::get_id();
    return idoss.str();
}
std::string Logger::getCurrentTime()
{
    auto now = std::chrono::system_clock::now();
    std::time_t t = std::chrono::system_clock::to_time_t(now);
    std::tm tm_time;
#if defined(_WIN32) || defined(_WIN64)
    localtime_s(&tm_time, &t);
#else
    localtime_r(&t, &tm_time);
#endif
    timeoss.clear();
    timeoss << std::put_time(&tm_time, "%Y-%m-%d %H:%M:%S");
    return timeoss.str();
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
    if (filenumber >= loggerconfig.maxfilenumbers)
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