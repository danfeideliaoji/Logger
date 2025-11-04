# Logger
一个轻量级且高性能的cpp日志库，旨在让项目快速集成日志功能。
## 功能特性
高性能，1秒内可记录百万条日志记录

接口简单，用CMake构建跨平台，可直接使用。

支持 DEBUG / INFO / WARNING / ERROR 级别与过滤。

控制台与文件输出，支持文件轮换，限制文件数量。

日志格式化（时间、级别、消息、代码位置等）。

支持字符串输入与格式化输入。

线程安全，文件异常捕获，异步写入，线程缓冲提升并发性能。

无冗余拷贝，内存消耗小。

支持动态配置，提供多种配置选择。

##  配置说明

🔧 构建说明

本项目使用 CMake 3.10+ 进行构建，支持 C++17 标准。

📂 构建步骤

下载Logger到项目根目录，在项目根目录外CMakeLists文件夹（与 Logger 同级）：

新创的CMakeLists加上

```
add_subdirectory(Logger)

target_link_libraries(main PRIVATE Logger)
```

添加后完成下面三步就完成构建
```
mkdir build

cd build

cmake ..
```

说明：


生成的静态库位于：

Linux/macOS: build/lib/libLogger.a

Windows: build/lib/Logger.lib

一个简单CMakeLists演示
```
cmake_minimum_required(VERSION 3.10)
project(main)
set(CMAKE_CXX_STANDARD 17)
add_executable(main main.cpp)
add_subdirectory(Logger)
target_link_libraries(main PRIVATE Logger)
```

## 性能演示

注：下面演示是vs2022 release配置 单位毫秒

### 日志双线程总共输出100万次所用时间
<img width="1730" height="424" alt="image" src="https://github.com/user-attachments/assets/e9ea64ae-2457-4005-bee6-790f88883dfe" />

### 直接输出数字从1到100万所用时间
<img width="1730" height="424" alt="image" src="https://github.com/user-attachments/assets/6aaafd7e-5cb0-4775-97ae-9bd947255bc0" />

