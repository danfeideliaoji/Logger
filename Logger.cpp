#include<string>
#include<chrono>
#include<ctime>
#include<sstream>
#include<iomanip>
#include<fstream>
#include<mutex>
#include<iostream>
#include<thread>
#include<filesystem>
#include"Logger.h"
Logger::Logger(){
    using namespace std::filesystem;
    if(!exists("./logs")){
        create_directory("./logs");
    }
    fileCreate();
}
void Logger::log(const std::string& message,LogLevel level,const char *file,int line){
    std::unique_lock<std::mutex> lock(logmutex);
    if(level<loglevel){
        return;
    }
    auto now= std::chrono::system_clock::now();
    if(now-lasttimeupdate>=std::chrono::seconds(1)){
        lasttimeupdate=now;
        currenttime=getCurrentTime();
    }
    if(outputmode&OutPutMode::CONSOLE){
        consoleLog(message,currenttime,level,file,line);
    }
    if(outputmode&OutPutMode::FILE){
        fileLog(message,currenttime,level,file,line);
    }
}
void Logger::consoleLog(const std::string& message,const std::string& currenttime,
    LogLevel level,const char *file,int line){
    std::cout<<"["<<currenttime<<"]"<<"["<<std::this_thread::get_id()<<"]"
    <<"["<<file<<":"<<line<<"]"
    <<"["<<logstring<<"]:"
    <<message<<std::endl;
}
void Logger::fileCreate(){
    if(!(outputmode&OutPutMode::FILE)){
        return;
    }
    std::ofstream file(logfile,std::ios::app);
    if(!file.is_open()){
        file.open(logfile,std::ios::out);   
    }
    file.close();
    logstream.open(logfile,std::ios::app);
    if(!logstream.is_open()){
        throw std::runtime_error("无法打开日志文件");
        exit(EXIT_FAILURE);
    }    
}
void Logger::fileLog(const std::string& message,const std::string& currenttime,
    LogLevel level,const char *file,int line){
    switch (level){
        case LogLevel::DEBUG:
            logstring="DEBUG";
            break;
        case LogLevel::INFO:
            logstring="INFO";
            break;
        case LogLevel::WARN:
             logstring="WARN";
            break;
        case LogLevel::ERROR:
             logstring="ERROR";
            break;
    }
    std::string filestring="["+currenttime+"]"+"["+getCurrentId()+"]"
    +"["+file+":"+std::to_string(line)+"]"+"message: "+message+"\n";
    currentfilebyte+=filestring.size();
    if(currentfilebyte>=maxfilebyte){
        logRotate();
        currentfilebyte=filestring.size();
        if(currentfilebyte>=maxfilebyte){
            throw std::runtime_error("单条日志超过最大文件大小限制");
            exit(EXIT_FAILURE);
        }
    }
    logstream<<filestring;
}
std::string Logger::getCurrentId(){
    std::ostringstream oss;
    oss<<std::this_thread::get_id();
    return oss.str();
}
std::string Logger::getCurrentTime(){
       auto now= std::chrono::system_clock::now();
       std::time_t t=std::chrono::system_clock::to_time_t(now);
       std::tm tm_time;
    #if defined(_WIN32) || defined(_WIN64)
        localtime_s(&tm_time,&t);
    #else 
        localtime_t(&t,&tm_time);
    #endif
    std::ostringstream oss;
    oss << std::put_time(&tm_time,"%Y-%m-%d %H:%M:%S");
    return oss.str();
}
void Logger::logRotate(){
    logstream.close();
    filenumber++;
    logUpdateDelete();
    fileCreate();
}
void Logger::logUpdateDelete(){
    using namespace std::filesystem;
    if(filenumber>=maxfilenumber){
        remove(logfile + "." + std::to_string(filenumber - 1));
        filenumber--;
    }
    for(int i=filenumber-1;i>=0;--i){
        std::string oldname;
        if(i==0){
             oldname=logfile;
        }
        else
             oldname=logfile + "." + std::to_string(i);
        std::string newname=logfile + "." + std::to_string(i + 1);
        if(exists(oldname)){
            rename(oldname,newname);
        }
    }
}