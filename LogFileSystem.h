#pragma once
#include "common.h"
class LogFileSystem {
public:
    LogFileSystem( struct LoggerConfig &loggerconfig);
    ~LogFileSystem();
    size_t logsInit(bool keep);
    void logRotate(std::ofstream &logstream);
    void fileCreate(std::ofstream &logstream);
private:
    void initFromConfig();
    void logUpdateDelete();
    struct LoggerConfig &loggerconfig;
    size_t filenumber;
    size_t maxfilenumbers;
    OutPutMode outputmode;
};