#include "LogFileSystem.h"
#include<filesystem>
#include<iostream>
#include<fstream>
LogFileSystem::LogFileSystem()
{
    loggerconfig=std::atomic_load(&loggerConfig);
}

LogFileSystem::~LogFileSystem()
{
}
size_t LogFileSystem::logsInit()
{
    using namespace std::filesystem;
    path logdir=path(PROJECT_SOURCE_DIR)/"/logs";
    if (loggerconfig->keeplastlogs)
    {
        if (!exists(logdir))
        {
            try{
                create_directory(logdir);
            }
            catch(const filesystem_error& e){
                std::cerr<<"创建日志目录失败: "<<e.what()<<std::endl;
            }
            filenumber = 0;
            return 0;
        }
        else
        {
            for (auto &entry : directory_iterator(logdir))
            {
                if (is_regular_file(entry.path()))
                {
                    filenumber++;
                }
            }
            if (exists(loggerconfig->logfile))
                return file_size(loggerconfig->logfile);
            else
                return 0;
        }
    }
    else
    {
        if (exists(logdir))
        {
            try
            {
                remove_all(logdir);
            }
            catch (const filesystem_error &e)
            {
                std::cerr << "删除失败: " << e.what() << std::endl;
            }
            try{
                create_directory(logdir); 
            }
            catch(const filesystem_error& e){
                std::cerr<<"创建日志目录失败: "<<e.what()<<std::endl;
            }
            
        }
        return 0;
    }
}

void LogFileSystem::logRotate(std::ofstream &logstream)
{   
    loggerconfig=std::atomic_load(&loggerConfig);
    logstream.close();
    filenumber++;
    logUpdateDelete();
    fileCreate(logstream);
}
void LogFileSystem::logUpdateDelete()
{
    using namespace std::filesystem;
    while (filenumber >= loggerconfig->maxfilenumbers)
    {
       try{
        remove(loggerconfig->logfile + "." + std::to_string(filenumber - 1));
       }
       catch (const filesystem_error &e)
       {
           std::cerr << "删除日志失败: " << e.what() << std::endl;
       } 
        filenumber--;
    }
    for (size_t i = filenumber - 1; i >= 0; --i)
    {
        std::string oldname;
        if (i == 0)
        {
            oldname = loggerconfig->logfile;
        }
        else
            oldname = loggerconfig->logfile + "." + std::to_string(i);
        std::string newname = loggerconfig->logfile + "." + std::to_string(i + 1);
        if (exists(oldname))
        {
            try{
                rename(oldname, newname);
            }
            catch (const filesystem_error &e)
            {
                std::cerr << "日志重命名失败: " << e.what() << std::endl;
            }
        }
    }
}
void LogFileSystem::fileCreate(std::ofstream &logstream)
{
    if (!(loggerconfig->outputmode & OutPutMode::FILE))
    {
        return;
    }
    logstream.open(loggerconfig->logfile, std::ios::app);
    if (!logstream.is_open())
    {
        logstream.open(loggerconfig->logfile, std::ios::out);
    }
}