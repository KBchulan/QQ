#pragma once

#include <unordered_map>
#include <memory>
#include <mutex>
#include "../core/User.h"
#include "DatabaseManager.h"

class UserManager {
private:
    std::unordered_map<int64_t, std::shared_ptr<User>> onlineUsers_;
    std::mutex usersMutex_;
    static UserManager instance_;

    UserManager() {}

public:
    static UserManager& getInstance() {
        static UserManager instance;
        return instance;
    }

    // 用户注册
    bool registerUser(const std::string& username, 
                     const std::string& password,
                     const std::string& nickname = "");

    // 用户登录
    bool login(const std::string& username, 
              const std::string& password,
              int64_t& userId);

    // 用户登出
    bool logout(int64_t userId);

    // 获取用户信息
    std::shared_ptr<User> getUser(int64_t userId);
    std::shared_ptr<User> getUserByUsername(const std::string& username);

    // 更新用户信息
    bool updateUserInfo(const User& user);

    // 检查用户是否在线
    bool isUserOnline(int64_t userId);

    // 获取在线用户列表
    std::vector<std::shared_ptr<User>> getOnlineUsers();

private:
    std::string hashPassword(const std::string& password);
    bool validatePassword(const std::string& password, const std::string& hash);
}; 