#include "../include/task_factory.h"

#include <memory>


    std::unique_ptr<Task> taskFactory::create_task(TaskType type, const std::string &name){

        switch (type) {
            case TaskType::COUNT:
                return std::make_unique<countTask>(name,1000);
            case TaskType::SUM:
                return std::make_unique<sumTask>(name,0,100);
            default:
                std::cerr << "[TaskFactory] 未知任务类型!" << std::endl;
                return nullptr;
        }

    }