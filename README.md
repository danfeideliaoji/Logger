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

## 快速开始
```
#include "Logger.h"

int main() {
    LOG_INFO("Hello Logger!"); //直接日志输出 默认是文件
    LOGF_DEBUG("Debug number: %d", 42);//格式化输出
    LOG_CWARNING("Warning");//C代表console 表示这条日志与配置无关 直接输出到控制台
    LOGF_FERROR("Error code: %d", -1);//F代表file 表示日志与配置无关 直接输出文件
    //下面是自定义配置 也可直接使用配置
    LOG_SET_LEVEL(LogLevel INFO) //表示INFO以下的消息不需要输出
    LOG_SET_MAXFILENUM(20)//表示日志获得最大文件数为20
    LOG_SET_OUTPUTMODE(OutPutMode Console)//设置输出到控制台
    LOG_GET_MAXFILENUM()//获得最大的文件数
    LOG_GET_LEVEL()// 获得过滤level
    //其他配置请看Logger.h 和 common.h
    return 0;
}
```
## 🧭 未来计划（如果打算升级的话）
🚀 性能与系统优化

 支持多文件异步写入（不同级别写入不同文件）

 优化异步队列机制，减少线程切换开销

 增加无锁队列版本，进一步提升并发性能

 支持日志压缩与分块写入

⚙️ 功能扩展

 支持自定义日志格式模板（例如：[{time}] [{level}] {msg}）

 控制台输出颜色高亮（不同级别显示不同颜色）

 添加日志统计（实时显示写入速度、丢失率等）

 支持远程日志上传（HTTP / TCP / UDP）

 支持 JSON / CSV 输出格式，方便日志分析

💡 开发与文档

 提供更详细的 API 文档与示例

