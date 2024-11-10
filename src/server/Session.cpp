#include "Session.h"
#include "Logger.h"
#include "UserManager.h"
#include "DatabaseManager.h"
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
    auto self(shared_from_this());
    boost::asio::async_read(socket_,
        boost::asio::buffer(&messageLength_, sizeof(messageLength_)),
        [this, self](const boost::system::error_code& error, size_t bytes_transferred) {
            if (!error) {
                messageBuffer_.resize(messageLength_);
                boost::asio::async_read(socket_,
                    boost::asio::buffer(messageBuffer_),
                    [this, self](const boost::system::error_code& error,
                                size_t bytes_transferred) {
                        handleRead(error, bytes_transferred);
                    });
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
    LOG_INFO("处理消息类型: " + std::to_string(static_cast<int>(msg.getType())));
    
    switch (msg.getType()) {
        case MessageType::LOGIN: {
            LOG_INFO("收到登录请求");
            Json::Value loginData;
            Json::Reader reader;
            if (!reader.parse(msg.getContent(), loginData)) {
                LOG_ERROR("解析登录信息失败: " + msg.getContent());
                sendLoginResponse(false, "无效的登录数据", 0);
                return;
            }

            std::string username = loginData["username"].asString();
            std::string password = loginData["password"].asString();
            LOG_INFO("尝试登录 - 用户名: " + username);

            try {
                int64_t userId;
                bool success = UserManager::getInstance().login(username, password, userId);
                if (success) {
                    LOG_INFO("用户登录成功: " + username + ", ID: " + std::to_string(userId));
                    authenticated_ = true;
                    userId_ = userId;
                    sendLoginResponse(true, "", userId);
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
    Json::Value response;
    response["success"] = success;
    if (success) {
        response["userId"] = userId;
    } else {
        response["error"] = error;
    }

    Message responseMsg(0, 0, response.toStyledString(), MessageType::LOGIN_RESPONSE);
    LOG_INFO("发送登录响应: " + (success ? "成功" : "失败 - " + error));
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

bool Session::isAlive() const {
    return socket_.is_open() && 
           (std::time(nullptr) - lastHeartbeat_ <= RECONNECT_TIMEOUT);
} 