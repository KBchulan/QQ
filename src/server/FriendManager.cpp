#include "FriendManager.h"

std::vector<std::shared_ptr<User>> FriendManager::getFriendList(int64_t userId) {
    std::lock_guard<std::mutex> lock(friendsMutex_);
    
    // 如果缓存中没有，从数据库加载
    if (friendsMap_.find(userId) == friendsMap_.end()) {
        loadFriendList(userId);
    }
    
    return friendsMap_[userId];
}

bool FriendManager::sendFriendRequest(int64_t fromUserId, int64_t toUserId) {
    LOG_INFO("发送好友请求: " + std::to_string(fromUserId) + " -> " + std::to_string(toUserId));
    
    // 检查是否已经是好友
    if (areFriends(fromUserId, toUserId)) {
        LOG_WARNING("已经是好友关系");
        return false;
    }
    
    // 构造SQL
    std::stringstream ss;
    ss << "INSERT INTO friendships (user_id, friend_id, status) VALUES ("
       << fromUserId << ", " << toUserId << ", 0)";
       
    bool success = DatabaseManager::getInstance().executeQuery(ss.str());
    if (success) {
        LOG_INFO("好友请求发送成功");
    } else {
        LOG_ERROR("好友请求发送失败");
    }
    
    return success;
}

bool FriendManager::handleFriendRequest(int64_t fromUserId, int64_t toUserId, bool accept) {
    LOG_INFO("处理好友请求: " + std::to_string(fromUserId) + " -> " + 
             std::to_string(toUserId) + (accept ? " 接受" : " 拒绝"));
    
    std::stringstream ss;
    ss << "UPDATE friendships SET status = " << (accept ? "1" : "2")
       << " WHERE user_id = " << fromUserId 
       << " AND friend_id = " << toUserId;
       
    bool success = DatabaseManager::getInstance().executeQuery(ss.str());
    
    if (success && accept) {
        // 如果接受，创建双向好友关系
        ss.str("");
        ss << "INSERT INTO friendships (user_id, friend_id, status) VALUES ("
           << toUserId << ", " << fromUserId << ", 1)";
        success = DatabaseManager::getInstance().executeQuery(ss.str());
        
        if (success) {
            // 更新缓存
            updateFriendCache(fromUserId);
            updateFriendCache(toUserId);
        }
    }
    
    return success;
}

bool FriendManager::areFriends(int64_t userId1, int64_t userId2) {
    std::stringstream ss;
    ss << "SELECT COUNT(*) FROM friendships WHERE "
       << "user_id = " << userId1 
       << " AND friend_id = " << userId2
       << " AND status = 1";
       
    MYSQL_RES* result = DatabaseManager::getInstance().executeQueryWithResult(ss.str());
    if (!result) return false;
    
    MYSQL_ROW row = mysql_fetch_row(result);
    bool areFriends = (row && std::stoi(row[0]) > 0);
    mysql_free_result(result);
    
    return areFriends;
}

void FriendManager::loadFriendList(int64_t userId) {
    LOG_INFO("加载用户 " + std::to_string(userId) + " 的好友列表");
    
    std::stringstream ss;
    ss << "SELECT u.user_id, u.username, u.nickname, u.status "
       << "FROM users u INNER JOIN friendships f ON u.user_id = f.friend_id "
       << "WHERE f.user_id = " << userId << " AND f.status = 1";
       
    MYSQL_RES* result = DatabaseManager::getInstance().executeQueryWithResult(ss.str());
    if (!result) return;
    
    std::vector<std::shared_ptr<User>> friends;
    MYSQL_ROW row;
    while ((row = mysql_fetch_row(result))) {
        auto friend_ = std::make_shared<User>();
        friend_->setUserId(std::stoll(row[0]));
        friend_->setUsername(row[1]);
        friend_->setNickname(row[2] ? row[2] : row[1]);
        friend_->setOnline(std::stoi(row[3]) == 1);
        friends.push_back(friend_);
    }
    
    mysql_free_result(result);
    friendsMap_[userId] = friends;
    
    LOG_INFO("加载了 " + std::to_string(friends.size()) + " 个好友");
}

void FriendManager::updateFriendCache(int64_t userId) {
    LOG_INFO("更新用户 " + std::to_string(userId) + " 的好友缓存");
    
    std::lock_guard<std::mutex> lock(friendsMutex_);
    
    // 从数据库重新加载好友列表
    std::stringstream ss;
    ss << "SELECT u.user_id, u.username, u.nickname, u.status "
       << "FROM users u INNER JOIN friendships f ON u.user_id = f.friend_id "
       << "WHERE f.user_id = " << userId << " AND f.status = 1";
       
    MYSQL_RES* result = DatabaseManager::getInstance().executeQueryWithResult(ss.str());
    if (!result) {
        LOG_ERROR("更新好友缓存失败");
        return;
    }
    
    std::vector<std::shared_ptr<User>> friends;
    MYSQL_ROW row;
    while ((row = mysql_fetch_row(result))) {
        auto friend_ = std::make_shared<User>();
        friend_->setUserId(std::stoll(row[0]));
        friend_->setUsername(row[1]);
        friend_->setNickname(row[2] ? row[2] : row[1]);
        friend_->setOnline(std::stoi(row[3]) == 1);
        friends.push_back(friend_);
    }
    
    mysql_free_result(result);
    friendsMap_[userId] = friends;
    
    LOG_INFO("好友缓存更新完成，共 " + std::to_string(friends.size()) + " 个好友");
} 