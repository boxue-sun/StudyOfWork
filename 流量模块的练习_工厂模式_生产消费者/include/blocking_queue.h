#ifndef BLOCK_QUEUE_H
#define BLOCK_QUEUE_H

#include <condition_variable>
#include <iostream>
#include <deque>
#include <mutex>
#include <string>

template <typename T>
class BlockingQueue{
public:

   explicit BlockingQueue( size_t cap = 100):_cap(cap){

    }
    ~BlockingQueue(){

    }

    void put(const T&item){
        std::unique_lock<std::mutex> lock(_mutex);
        queue.push_back(item);

        //如果超出容量
        if(_cap>0&&queue.size()>_cap){
            queue.pop_front();
        }

        lock.unlock();
        //唤醒
        _not_empty.notify_one();

        
    }

    void put(T&& item) {
    std::unique_lock<std::mutex> lock(_mutex);
    queue.push_back(std::move(item)); // 移动而非拷贝
    // ... 容量检查和唤醒逻辑
}

    //消费端
    T take(){
        std::unique_lock<std::mutex> lock(_mutex);
        //上锁
        //是否有数据
        while(queue.empty()){
            _not_empty.wait(lock);
        }

         T front = std::move(queue.front());
         queue.pop_front();
         return  front;

    }

    size_t size() const{
        std::lock_guard<std::mutex> lock(_mutex);
        return queue.size();
    }

    bool empty() const{
        std::lock_guard<std::mutex> lock(_mutex);
        return queue.empty();
    }

private:
    std::deque<T> queue;
    mutable std::mutex _mutex;
    std::condition_variable _not_empty;

    size_t _cap;

};



#endif