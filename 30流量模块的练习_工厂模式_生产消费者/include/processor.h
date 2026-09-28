#ifndef PROCESSOR_H
#define PROCESSOR_H

#include "task.h"
#include <algorithm>
#include <atomic>
#include <string>
#include <thread>
#include "blocking_queue.h"
//处理器状态

enum class ProcessorStatus{

    IDLE,     // 空闲
    RUNNING,  // 运行中
    EXIT      // 退出
};

class processor{
    public:
        processor();
        ~processor();
        //提交任务
        void sub(TaskType type,const std::string &name);

        //启动工作线程
        void start();

        //停止工作线程
        void stop();


    private:
        void work_loop();//主线程

    private:
        //原子变量记录状态
        std::atomic<ProcessorStatus> _status;
        BlockingQueue<std::unique_ptr<Task>>  _queue;
        //工作现场
        std::unique_ptr<std::thread> _worker;

};





#endif