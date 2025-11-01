#pragma once
#include <string>
#include <fstream>
#include <mutex>
#include <chrono>
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
class Logger
{
public:
    Logger();
    ~Logger() = default;
    static Logger &Instance()
    {
        static Logger instance;
        return instance;
    }
    void log(const std::string &message, LogLevel level, const char *file, int line);

public:
    std::string logfile = "./logs/log.txt";
    LogLevel loglevel = LogLevel::DEBUG;
    OutPutMode outputmode = OutPutMode::FILE;
    std::size_t maxfilebyte = 10 * 1024 * 1024; // 10 MB
    std::size_t filebyte=1;
    std::size_t maxfilenumber=10;
private:
    std::string getCurrentTime();
    std::string getCurrentId();
    void fileCreate();
    void consoleLog(const std::string &message, const std::string &currenttime,
                    LogLevel level, const char *file, int line);
    void fileLog(const std::string &message, const std::string &currenttime,
                 LogLevel level, const char *file, int line);
    void logRotate();
    void logUpdateDelete();             

private:
    std::ofstream logstream;
    std::mutex logmutex;
    std::string logstring;
    std::size_t currentfilebyte = 0;
    std::size_t filenumber = 1;
    std::string currenttime;
    std::chrono::time_point<std::chrono::system_clock> lasttimeupdate;
};

void inline LOG_SET_LEVEL(LogLevel level)
{
    Logger::Instance().loglevel = level;
}
void inline LOG_SET_FILE(const std::string &filepath)
{
    Logger::Instance().logfile = filepath;
}
void inline LOG_SET_OUTPUTMODE(OutPutMode mode)
{
    Logger::Instance().outputmode = mode;
}
void inline LOG_SET_MAXFILEBYTE(std::size_t size)
{
    Logger::Instance().maxfilebyte = size;
}
void inline LOG_SET_MAXFILENUMBER(std::size_t number)
{
    Logger::Instance().maxfilenumber = number;
}
size_t inline LOG_GET_FILENUMBER()
{
    return Logger::Instance().maxfilenumber;
}
#define LOG_DEBUG(message) Logger::Instance().log(message, LogLevel::DEBUG, __FILE__, __LINE__);
#define LOG_INFO(message) Logger::Instance().log(message, LogLevel::INFO, __FILE__, __LINE__);
#define LOG_WARN(message) Logger::Instance().log(message, LogLevel::WARN, __FILE__, __LINE__);
#define LOG_ERROR(message) Logger::Instance().log(message, LogLevel::ERROR, __FILE__, __LINE__);
