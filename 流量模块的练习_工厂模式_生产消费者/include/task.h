#ifndef TASK_H
#define TASK_H 

#include <string>
#include <iostream>
//枚举 加class有作用域  更安全
enum class TaskType{
    COUNT,
    SUM,
    UNKNOWN
};


//任务基类
class Task{
public:
    explicit Task(const std::string &name):_name(name){

    }
    virtual ~Task() = default;

    virtual void execute() = 0;
    std::string get_name  ( )const{
        return _name;   
    }

    virtual TaskType get_type()const = 0;

private:
    std::string _name;
};

//子类

class countTask:public Task{
public:
    countTask(const std::string &name,int count):Task(name),_count(count){

    }
    //不写析构默认

    //重写虚函数
    void execute() override{
        int sum = 0;
        for (int i = 0; i < _count; ++i) {
            sum += 1;
        }
        std::cout << "[CountTask] " << get_name()
                  << " 计算结果: " << sum << std::endl;

    }
    TaskType get_type()const override{
        
        return  TaskType::COUNT;
    }

private:
    int _count;
};

class sumTask:public Task{
public:
    sumTask(const std::string &name,int from,int to):Task(name),_from(from),_to(to){

    }
    
    void execute() override{
        int sum = 0;
        for (int i = _from; i <= _to; ++i) {
            sum += i;
        }
        std::cout << "[SumTask] " << get_name()
                  << " 计算结果: " << sum << std::endl;
    }

    TaskType get_type() const override{
        return  TaskType::SUM;
    }


private:
    int _from;
    int _to;
};


#endif
