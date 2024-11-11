#include "Session.h"
#include "Logger.h"
#include "UserManager.h"
#include "DatabaseManager.h"
#include "MessageManager.h"
#include "Server.h"
#include <iostream>

Session::Session(boost::asio::ip::tcp::socket socket)
    : socket_(std::move(socket))
    , authenticated_(false)
    , lastHeartbeat_(std::time(nullptr)) {
}

void Session::start() {
    startRead();
    // 启动心跳检测
    auto self(shared_from_this());
    boost::asio::steady_timer timer(socket_.get_executor());
    timer.expires_after(std::chrono::seconds(HEARTBEAT_INTERVAL));
    timer.async_wait([this, self](const boost::system::error_code& error) {
        if (!error) {
            checkHeartbeat();
        }
    });
}

void Session::startRead() {
    LOG_INFO("开始读取消息");
    
    // 先读取消息长度
    boost::asio::async_read(socket_,
        boost::asio::buffer(&messageLength_, sizeof(messageLength_)),
        [this](const boost::system::error_code& error, size_t bytes_transferred) {
            if (!error) {
                LOG_INFO("收到消息头，长度: " + std::to_string(messageLength_));
                messageBuffer_.resize(messageLength_);
                
                // 读取消息内容
                boost::asio::async_read(socket_,
                    boost::asio::buffer(messageBuffer_),
                    [this](const boost::system::error_code& error, size_t bytes_transferred) {
                        handleRead(error, bytes_transferred);
                    });
            } else {
                LOG_ERROR("读取消息长度失败: " + error.message());
                socket_.close();
            }
        });
}

void Session::handleRead(const boost::system::error_code& error,
                        size_t bytes_transferred) {
    if (!error) {
        // 解析消息
        std::string jsonStr(messageBuffer_.begin(), messageBuffer_.end());
        Json::Value root;
        Json::Reader reader;
        if (reader.parse(jsonStr, root)) {
            Message msg = Message::fromJson(root);
            processMessage(msg);
        } else {
            LOG_ERROR("解析消息失败: " + jsonStr);
        }
        
        // 继续读取下一条消息
        startRead();
    } else {
        LOG_ERROR("读取消息失败: " + error.message());
        socket_.close();
    }
}

void Session::checkHeartbeat() {
    auto now = std::time(nullptr);
    if (now - lastHeartbeat_ > HEARTBEAT_INTERVAL) {
        sendHeartbeat();
    }
    
    if (now - lastHeartbeat_ > RECONNECT_TIMEOUT) {
        socket_.close();
    }
}

void Session::sendHeartbeat() {
    // 实现心跳包发送
}

void Session::processMessage(const Message& msg) {
    LOG_INFO("收到消息，类型: " + std::to_string(static_cast<int>(msg.getType())));
    
    switch (msg.getType()) {
        case MessageType::LOGIN: {
            LOG_INFO("处理登录请求");
            Json::Value loginData;
            Json::Reader reader;
            if (!reader.parse(msg.getContent(), loginData)) {
                LOG_ERROR("解析登录信息失败: " + msg.getContent());
                sendLoginResponse(false, "无效的登录数据", 0);
                return;
            }

            std::string username = loginData["username"].asString();
            std::string password = loginData["password"].asString();
            LOG_INFO("登录尝试 - 用户名: " + username);

            try {
                int64_t userId;
                // 验证用户名和密码
                if (DatabaseManager::getInstance().authenticateUser(username, password, userId)) {
                    LOG_INFO("用户登录成功: " + username + " (ID: " + std::to_string(userId) + ")");
                    authenticated_ = true;
                    userId_ = userId;
                    
                    // 更新用户状态
                    DatabaseManager::getInstance().updateUserStatus(userId, true);
                    
                    // 发送成功响应
                    sendLoginResponse(true, "", userId);
                    
                    // 发送离线消息
                    auto offlineMessages = MessageManager::getInstance().getOfflineMessages(userId);
                    for (const auto& msg : offlineMessages) {
                        sendMessage(msg);
                    }
                    MessageManager::getInstance().clearOfflineMessages(userId);
                } else {
                    LOG_WARNING("用户登录失败: " + username);
                    sendLoginResponse(false, "用户名或密码错误", 0);
                }
            } catch (const std::exception& e) {
                LOG_ERROR("登录过程发生错误: " + std::string(e.what()));
                sendLoginResponse(false, "登录过程发生错误", 0);
            }
            break;
        }
        case MessageType::CHAT: {
            LOG_INFO("收到聊天消息");
            
            // 存储消息
            if (!MessageManager::getInstance().storeMessage(msg)) {
                LOG_ERROR("消息存储失败");
                return;
            }
            
            // 获取接收者的会话
            int64_t receiverId = msg.getReceiverId();
            auto receiverSession = Server::getInstance().getSession(receiverId);
            
            if (receiverSession && receiverSession->isAlive()) {
                // 如果接收者在线，直接转发消息
                receiverSession->sendMessage(msg);
                LOG_INFO("消息已转发给在线用户");
            } else {
                // 如果接收者离线，存储为离线消息
                MessageManager::getInstance().addOfflineMessage(receiverId, msg);
                LOG_INFO("消息已存储为离线消息");
            }
            break;
        }
        case MessageType::GET_CHAT_HISTORY: {
            LOG_INFO("收到获取聊天历史请求");
            
            Json::Value data;
            Json::Reader reader;
            if (!reader.parse(msg.getContent(), data)) {
                LOG_ERROR("解析请求数据失败");
                return;
            }
            
            int64_t otherUserId = data["otherUserId"].asInt64();
            auto messages = MessageManager::getInstance().getChatHistory(userId_, otherUserId);
            
            // 发送聊天历史
            Json::Value response;
            Json::Value messageArray(Json::arrayValue);
            for (const auto& msg : messages) {
                messageArray.append(msg.toJson());
            }
            response["messages"] = messageArray;
            
            Message responseMsg(0, userId_, response.toStyledString(), 
                              MessageType::CHAT_HISTORY_RESPONSE);
            sendMessage(responseMsg);
            break;
        }
        case MessageType::FRIEND_REQUEST: {
            LOG_INFO("收到好友请求");
            Json::Value requestData;
            Json::Reader reader;
            if (!reader.parse(msg.getContent(), requestData)) {
                LOG_ERROR("解析好友请求失败");
                return;
            }

            int64_t fromUserId = requestData["from_user_id"].asInt64();
            std::string toUsername = requestData["to_username"].asString();
            
            // 获取目标用户ID
            auto toUser = UserManager::getInstance().getUserByUsername(toUsername);
            if (!toUser) {
                LOG_WARNING("目标用户不存在: " + toUsername);
                sendFriendRequestResponse(false, "用户不存在", fromUserId);
                return;
            }

            // 发送好友请求通知给目标用户
            Json::Value notification;
            notification["type"] = "friend_request";
            notification["from_user_id"] = fromUserId;
            notification["from_username"] = UserManager::getInstance().getUser(fromUserId)->getUsername();
            
            Message notifyMsg(fromUserId, toUser->getUserId(), 
                            notification.toStyledString(), 
                            MessageType::FRIEND_REQUEST_NOTIFICATION);

            // 如果目标用户在线，直接发送通知
            auto targetSession = Server::getInstance().getSession(toUser->getUserId());
            if (targetSession && targetSession->isAlive()) {
                targetSession->sendMessage(notifyMsg);
                LOG_INFO("已发送好友请求通知给用户: " + toUsername);
            } else {
                // 存储离线通知
                MessageManager::getInstance().addOfflineMessage(toUser->getUserId(), notifyMsg);
                LOG_INFO("已存储离线好友请求通知给用户: " + toUsername);
            }

            // 发送响应给请求方
            sendFriendRequestResponse(true, "", fromUserId);
            break;
        }
        // ... 其他消息处理 ...
    }
}

void Session::sendRegistrationResponse(bool success, const std::string& error) {
    Json::Value response;
    response["success"] = success;
    if (!success && !error.empty()) {
        response["error"] = error;
    }

    Message responseMsg(0, 0, response.toStyledString(), MessageType::REGISTER_RESPONSE);
    LOG_INFO("发送注册响应: " + (success ? "成功" : "失败 - " + error));
    sendMessage(responseMsg);
}

void Session::sendLoginResponse(bool success, const std::string& error, int64_t userId) {
    LOG_INFO("发送登录响应 - " + std::string(success ? "成功" : "失败"));
    
    Json::Value response;
    response["success"] = success;
    if (success) {
        response["userId"] = Json::Value::Int64(userId);
    } else {
        response["error"] = error;
    }

    Message responseMsg(0, 0, response.toStyledString(), MessageType::LOGIN_RESPONSE);
    sendMessage(responseMsg);
}

void Session::sendMessage(const Message& msg) {
    try {
        // 序列化消息
        Json::Value jsonMsg = msg.toJson();
        Json::FastWriter writer;
        std::string message = writer.write(jsonMsg);

        // 发送消息长度
        uint32_t messageLength = message.length();
        boost::asio::write(socket_, boost::asio::buffer(&messageLength, sizeof(messageLength)));

        // 发送消息内容
        boost::asio::write(socket_, boost::asio::buffer(message));
        
        LOG_INFO("消息发送成功，长度: " + std::to_string(messageLength));
    } catch (const boost::system::system_error& e) {
        LOG_ERROR("发送消息失败: " + std::string(e.what()));
        socket_.close();
    }
}

void Session::sendFriendRequestResponse(bool success, const std::string& error, int64_t userId) {
    Json::Value response;
    response["success"] = success;
    if (!success) {
        response["error"] = error;
    }

    Message responseMsg(0, userId, response.toStyledString(), 
                       MessageType::FRIEND_REQUEST_RESPONSE);
    sendMessage(responseMsg);
}

bool Session::isAlive() const {
    return socket_.is_open() && 
           (std::time(nullptr) - lastHeartbeat_ <= RECONNECT_TIMEOUT);
} 