cmake_minimum_required(VERSION 3.10)
project(task_practice)

# 设置 C++17 标准（项目也用 C++17）
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 头文件目录
include_directories(include)

# 源文件
set(SOURCES
    src/main.cpp
    src/task_factory.cpp
    src/processor.cpp
)

# 生成可执行文件
add_executable(task_practice ${SOURCES})

# 链接线程库（Linux 上必须加 -pthread）
find_package(Threads REQUIRED)
target_link_libraries(task_practice Threads::Threads)