#include "MessageQueue.h"

void MessageQueue::push(const Message& msg) {
    std::lock_guard<std::mutex> lock(mutex_);
    queue_.push(msg);
    cv_.notify_one();
}

bool MessageQueue::pop(Message& msg) {
    std::unique_lock<std::mutex> lock(mutex_);
    
    // 等待直到队列非空或停止
    cv_.wait(lock, [this] { 
        return !queue_.empty() || stopping_; 
    });
    
    if (stopping_ && queue_.empty()) {
        return false;
    }
    
    msg = queue_.front();
    queue_.pop();
    return true;
}

void MessageQueue::stop() {
    std::lock_guard<std::mutex> lock(mutex_);
    stopping_ = true;
    cv_.notify_all();
}

bool MessageQueue::empty() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return queue_.empty();
} 