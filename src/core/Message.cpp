#include "Message.h"

Message::Message(int64_t senderId, int64_t receiverId, 
                const std::string& content, MessageType type)
    : messageId_(0)
    , senderId_(senderId)
    , receiverId_(receiverId)
    , type_(type)
    , content_(content)
    , timestamp_(std::time(nullptr)) {
}

Json::Value Message::toJson() const {
    Json::Value root;
    root["messageId"] = Json::Value::Int64(messageId_);
    root["senderId"] = Json::Value::Int64(senderId_);
    root["receiverId"] = Json::Value::Int64(receiverId_);
    root["type"] = static_cast<int>(type_);
    root["content"] = content_;
    root["timestamp"] = Json::Value::Int64(timestamp_);
    return root;
}

Message Message::fromJson(const Json::Value& json) {
    Message msg;
    msg.messageId_ = json["messageId"].asInt64();
    msg.senderId_ = json["senderId"].asInt64();
    msg.receiverId_ = json["receiverId"].asInt64();
    msg.type_ = static_cast<MessageType>(json["type"].asInt());
    msg.content_ = json["content"].asString();
    msg.timestamp_ = json["timestamp"].asInt64();
    return msg;
} 