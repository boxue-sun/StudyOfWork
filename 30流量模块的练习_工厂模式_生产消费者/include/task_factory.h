//静态工厂模式用于生成任务对象  
#ifndef TASK_FACTORY_H
#define TASK_FACTORY_H
#include "task.h"
#include <memory>


#include <algorithm>
class taskFactory{
public:
    static std::unique_ptr<Task> create_task(TaskType type,const std::string& name);


private:



};

#endif


