#pragma once
#include <string>
#include <ctime>

class User {
private:
    int64_t userId_;
    std::string username_;
    std::string nickname_;
    std::string avatarUrl_;
    bool online_;
    std::time_t lastLoginTime_;

public:
    User() : userId_(0), online_(false) {}
    
    void setUserId(int64_t id) { userId_ = id; }
    void setUsername(const std::string& username) { username_ = username; }
    void setNickname(const std::string& nickname) { nickname_ = nickname; }
    void setAvatarUrl(const std::string& url) { avatarUrl_ = url; }
    void setOnline(bool online) { online_ = online; }
    void setLastLoginTime(std::time_t time) { lastLoginTime_ = time; }

    int64_t getUserId() const { return userId_; }
    const std::string& getUsername() const { return username_; }
    const std::string& getNickname() const { return nickname_; }
    const std::string& getAvatarUrl() const { return avatarUrl_; }
    bool isOnline() const { return online_; }
    std::time_t getLastLoginTime() const { return lastLoginTime_; }
}; 