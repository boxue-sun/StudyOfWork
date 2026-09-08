#include "../include/processor.h"
#include "../include/task_factory.h"
#include <memory>


processor::processor():_status(ProcessorStatus::IDLE),_queue(50){

}

processor::~processor(){
    this->stop();

}

void processor::sub(TaskType type, const std::string &name){
    //提交类型
        //用工厂创建类型
            auto task  =  taskFactory::create_task(type, name);

        //
        if(task){
            _queue.put(std::move(task));
            std::cout << "[Producer] 提交任务: " << name << std::endl;
        }

}
void processor::start(){
    if(_status!=ProcessorStatus::IDLE){
        std::cerr << "[Processor] 不在 IDLE 状态，无法启动!" << std::endl;
        return;
    }

    _status = ProcessorStatus::RUNNING;//改状态

    _worker = std::make_unique<std::thread>(&processor::work_loop,this);
}
void processor::stop(){
    if(_status!= ProcessorStatus::RUNNING){
        return;
    }
    _status = ProcessorStatus::EXIT;

    //防止还有睡觉的线程 当队列为空就会睡觉 所以往队列塞个东西即可
    _queue.put(nullptr);

    //回收
    if(_worker &&_worker->joinable()){
        _worker->join();
        std::cout << "[Processor] 工作线程已退出" << std::endl;
    }
}

void processor::work_loop(){
        while(_status == ProcessorStatus::RUNNING){
            auto task = _queue.take();
            if(task == nullptr){
                std::cout << "[Worker] 收到退出信号" << std::endl;
                break;
            }
             // 执行任务
        std::cout << "[Worker] 处理任务: " << task->get_name()
                  << " (类型: " << static_cast<int>(task->get_type()) << ")"
                  << std::endl;

                   task->execute();
        }
       
}