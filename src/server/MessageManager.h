#pragma once

#include <vector>
#include <queue>
#include <mutex>
#include <memory>
#include <unordered_map>
#include "../core/Message.h"
#include "DatabaseManager.h"
#include "Logger.h"

class MessageManager {
private:
    static MessageManager instance_;
    std::mutex messageMutex_;
    
    // 用户ID到消息队列的映射（存储离线消息）
    std::unordered_map<int64_t, std::queue<Message>> offlineMessages_;

    MessageManager() {}

public:
    static MessageManager& getInstance() {
        static MessageManager instance;
        return instance;
    }

    // 存储消息
    bool storeMessage(const Message& msg);
    
    // 获取聊天历史
    std::vector<Message> getChatHistory(int64_t userId1, int64_t userId2, int limit = 50);
    
    // 获取离线消息
    std::vector<Message> getOfflineMessages(int64_t userId);
    
    // 添加离线消息
    void addOfflineMessage(int64_t userId, const Message& msg);
    
    // 清除离线消息
    void clearOfflineMessages(int64_t userId);

private:
    // 从数据库加载消息历史
    std::vector<Message> loadMessagesFromDb(int64_t userId1, int64_t userId2, int limit);
}; 