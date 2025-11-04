cmake_minimum_required(VERSION 3.10)
project(Logger)
set(CMAKE_CXX_STANDARD 17)
# 源文件
set(LOG_SOURCES
    src/Logger.cpp
    src/LogThreadLocal.cpp
    src/LogFileSystem.cpp
)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
# 创建静态库
add_library(Logger STATIC ${LOG_SOURCES})
# 定义项目根目录宏
target_compile_definitions(Logger PUBLIC PROJECT_SOURCE_DIR="${CMAKE_CURRENT_SOURCE_DIR}")
# 头文件目录
target_include_directories(Logger
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include
)
