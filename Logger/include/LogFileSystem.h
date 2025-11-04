#pragma once
#include "common.h"
class LogFileSystem {
public:
    LogFileSystem();
    ~LogFileSystem();
    size_t logsInit();
    void logRotate(std::ofstream &logstream);
    void fileCreate(std::ofstream &logstream);
private:
    void logUpdateDelete();
    std::shared_ptr<struct LoggerConfig>loggerconfig;
    size_t filenumber=0;
};