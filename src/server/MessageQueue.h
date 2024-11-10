#pragma once

#include <queue>
#include <mutex>
#include <condition_variable>
#include "../core/Message.h"

class MessageQueue {
private:
    std::queue<Message> queue_;
    std::mutex mutex_;
    std::condition_variable cv_;
    bool stopping_;

public:
    MessageQueue() : stopping_(false) {}
    
    void push(const Message& msg);
    bool pop(Message& msg);
    void stop();
    bool empty() const;
}; 