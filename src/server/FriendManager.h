#pragma once

#include <unordered_map>
#include <vector>
#include <memory>
#include <mutex>
#include "../core/User.h"
#include "DatabaseManager.h"
#include "Logger.h"

class FriendManager {
private:
    static FriendManager instance_;
    std::mutex friendsMutex_;
    
    // 用户ID到好友列表的映射
    std::unordered_map<int64_t, std::vector<std::shared_ptr<User>>> friendsMap_;

    FriendManager() {}

public:
    static FriendManager& getInstance() {
        static FriendManager instance;
        return instance;
    }

    // 获取好友列表
    std::vector<std::shared_ptr<User>> getFriendList(int64_t userId);
    
    // 添加好友请求
    bool sendFriendRequest(int64_t fromUserId, int64_t toUserId);
    
    // 处理好友请求
    bool handleFriendRequest(int64_t fromUserId, int64_t toUserId, bool accept);
    
    // 检查是否是好友
    bool areFriends(int64_t userId1, int64_t userId2);
    
    // 删除好友
    bool removeFriend(int64_t userId1, int64_t userId2);
    
    // 获取好友请求列表
    std::vector<std::shared_ptr<User>> getFriendRequests(int64_t userId);

private:
    // 从数据库加载好友列表
    void loadFriendList(int64_t userId);
    
    // 更新好友关系缓存
    void updateFriendCache(int64_t userId);
}; 