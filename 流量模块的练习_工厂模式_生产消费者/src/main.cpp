
#include <iostream>
#include "../include/processor.h"
#include <chrono>
#include <thread>


int main(){


    std::cout<<"===========================任务处理启动==========================="<<std::endl;
    processor pro;

    std::this_thread::sleep_for(std::chrono::milliseconds(100)); // 等一下让线程启动
    pro.start();

    pro.sub(TaskType::COUNT, "计数A");
    pro.sub(TaskType::SUM, "求和A");
    pro.sub(TaskType::COUNT, "计数b");
    pro.sub(TaskType::SUM, "求和Ac");
    std::this_thread::sleep_for(std::chrono::seconds(1));
    pro.stop();

std::cout << "===== 任务处理系统结束 =====" << std::endl;
    return 0;
}