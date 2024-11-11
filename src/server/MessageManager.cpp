#include "MessageManager.h"

bool MessageManager::storeMessage(const Message& msg) {
    LOG_INFO("存储消息 - 从用户" + std::to_string(msg.getSenderId()) + 
             "到用户" + std::to_string(msg.getReceiverId()));
    
    std::stringstream ss;
    ss << "INSERT INTO messages (sender_id, receiver_id, content, msg_type, send_time) VALUES ("
       << msg.getSenderId() << ", "
       << msg.getReceiverId() << ", '"
       << msg.getContent() << "', "
       << static_cast<int>(msg.getType()) << ", "
       << "NOW())";
       
    bool success = DatabaseManager::getInstance().executeQuery(ss.str());
    if (!success) {
        LOG_ERROR("消息存储失败");
        return false;
    }
    
    LOG_INFO("消息存储成功");
    return true;
}

std::vector<Message> MessageManager::getChatHistory(int64_t userId1, int64_t userId2, int limit) {
    LOG_INFO("获取聊天历史 - 用户" + std::to_string(userId1) + 
             "和用户" + std::to_string(userId2));
    
    std::stringstream ss;
    ss << "SELECT msg_id, sender_id, receiver_id, content, msg_type, send_time "
       << "FROM messages WHERE "
       << "((sender_id = " << userId1 << " AND receiver_id = " << userId2 << ") OR "
       << "(sender_id = " << userId2 << " AND receiver_id = " << userId1 << ")) "
       << "ORDER BY send_time DESC LIMIT " << limit;
       
    MYSQL_RES* result = DatabaseManager::getInstance().executeQueryWithResult(ss.str());
    if (!result) {
        LOG_ERROR("获取聊天历史失败");
        return std::vector<Message>();
    }
    
    std::vector<Message> messages;
    MYSQL_ROW row;
    while ((row = mysql_fetch_row(result))) {
        Message msg(
            std::stoll(row[1]),  // sender_id
            std::stoll(row[2]),  // receiver_id
            row[3],              // content
            static_cast<MessageType>(std::stoi(row[4]))  // msg_type
        );
        messages.push_back(msg);
    }
    
    mysql_free_result(result);
    LOG_INFO("获取到 " + std::to_string(messages.size()) + " 条消息");
    
    return messages;
}

void MessageManager::addOfflineMessage(int64_t userId, const Message& msg) {
    std::lock_guard<std::mutex> lock(messageMutex_);
    offlineMessages_[userId].push(msg);
    LOG_INFO("添加离线消息 - 用户" + std::to_string(userId));
}

std::vector<Message> MessageManager::getOfflineMessages(int64_t userId) {
    std::lock_guard<std::mutex> lock(messageMutex_);
    std::vector<Message> messages;
    
    auto it = offlineMessages_.find(userId);
    if (it != offlineMessages_.end()) {
        auto& queue = it->second;
        while (!queue.empty()) {
            messages.push_back(queue.front());
            queue.pop();
        }
    }
    
    LOG_INFO("获取 " + std::to_string(messages.size()) + 
             " 条离线消息 - 用户" + std::to_string(userId));
    return messages;
}

void MessageManager::clearOfflineMessages(int64_t userId) {
    std::lock_guard<std::mutex> lock(messageMutex_);
    offlineMessages_.erase(userId);
    LOG_INFO("清除离线消息 - 用户" + std::to_string(userId));
} 