#pragma once

#include <string>
#include <ctime>
#include <json/json.h>

enum class MessageType {
    TEXT,
    IMAGE,
    FILE,
    VOICE,
    VIDEO,
    SYSTEM,
    LOGIN,          // 登录消息
    LOGOUT,         // 登出消息
    REGISTER,       // 注册消息
    LOGIN_RESPONSE, // 登录响应
    ERROR,          // 错误消息
    HEARTBEAT,      // 心跳消息
    REGISTER_RESPONSE,  // 注册响应
};

class Message {
private:
    int64_t messageId_;
    int64_t senderId_;
    int64_t receiverId_;
    MessageType type_;
    std::string content_;
    std::time_t timestamp_;

public:
    Message() = default;
    Message(int64_t senderId, int64_t receiverId, const std::string& content, MessageType type = MessageType::TEXT);

    // Getters
    int64_t getMessageId() const { return messageId_; }
    int64_t getSenderId() const { return senderId_; }
    int64_t getReceiverId() const { return receiverId_; }
    MessageType getType() const { return type_; }
    const std::string& getContent() const { return content_; }
    std::time_t getTimestamp() const { return timestamp_; }

    // 序列化和反序列化
    Json::Value toJson() const;
    static Message fromJson(const Json::Value& json);
}; 