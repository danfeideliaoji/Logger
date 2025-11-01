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
    if (loggerconfig->keeplastlogs)
    {
        if (!exists("./logs"))
        {
            create_directory("./logs");
            filenumber = 0;
            return 0;
        }
        else
        {
            for (auto &entry : directory_iterator("logs"))
            {
                if (is_regular_file(entry.path()))
                {
                    filenumber++;
                }
            }
            if(filenumber==0) return 0;
            return file_size(loggerconfig->logfile);
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
        remove(loggerconfig->logfile + "." + std::to_string(filenumber - 1));
        filenumber--;
    }
    for (int i = filenumber - 1; i >= 0; --i)
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
            rename(oldname, newname);
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