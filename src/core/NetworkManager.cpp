#include "NetworkManager.h"
#include <iostream>

NetworkManager::NetworkManager()
    : socket_(io_context_)
    , isConnected_(false) {
}

NetworkManager::~NetworkManager() {
    if (isConnected_) {
        disconnect();
    }
}

bool NetworkManager::connect(const std::string& host, int port) {
    try {
        boost::asio::ip::tcp::endpoint endpoint(
            boost::asio::ip::address::from_string(host), port);
        
        socket_.connect(endpoint);
        isConnected_ = true;
        
        // 开始接收消息
        startReceiving();
        
        // 启动IO服务
        std::thread([this]() {
            try {
                io_context_.run();
            } catch (const std::exception& e) {
                std::cerr << "IO服务异常: " << e.what() << std::endl;
            }
        }).detach();
        
        return true;
    }
    catch (const boost::system::system_error& e) {
        std::cerr << "连接失败: " << e.what() << std::endl;
        return false;
    }
}

bool NetworkManager::disconnect() {
    if (!isConnected_) return false;
    
    try {
        socket_.close();
        isConnected_ = false;
        return true;
    }
    catch (const boost::system::system_error& e) {
        std::cerr << "断开连接失败: " << e.what() << std::endl;
        return false;
    }
}

void NetworkManager::sendMessage(const Message& msg) {
    if (!isConnected_) {
        std::cerr << "未连接到服务器" << std::endl;
        return;
    }

    try {
        // 序列化消息
        Json::Value jsonMsg = msg.toJson();
        Json::FastWriter writer;
        std::string message = writer.write(jsonMsg);

        // 先发送消息长度
        uint32_t messageLength = message.length();
        boost::asio::write(socket_, boost::asio::buffer(&messageLength, sizeof(messageLength)));

        // 发送消息内容
        boost::asio::write(socket_, boost::asio::buffer(message));
        
        std::cout << "消息发送成功，长度: " << messageLength << std::endl;
    } catch (const boost::system::system_error& e) {
        std::cerr << "发送消息失败: " << e.what() << std::endl;
        isConnected_ = false;
    }
}

void NetworkManager::startReceiving() {
    if (!isConnected_) return;

    std::cout << "开始接收消息..." << std::endl;
    
    // 启动异步读取
    boost::asio::async_read(socket_,
        boost::asio::buffer(&messageLength_, sizeof(messageLength_)),
        [this](const boost::system::error_code& error, size_t bytes_transferred) {
            if (!error) {
                std::cout << "收到消息头，长度: " << messageLength_ << std::endl;
                messageBuffer_.resize(messageLength_);
                boost::asio::async_read(socket_,
                    boost::asio::buffer(messageBuffer_),
                    [this](const boost::system::error_code& error, size_t bytes_transferred) {
                        if (!error) {
                            // 解析消息
                            std::string jsonStr(messageBuffer_.begin(), messageBuffer_.end());
                            std::cout << "收到完整消息: " << jsonStr << std::endl;
                            
                            Json::Value root;
                            Json::Reader reader;
                            if (reader.parse(jsonStr, root)) {
                                Message msg = Message::fromJson(root);
                                {
                                    std::lock_guard<std::mutex> lock(receiveMutex_);
                                    receivedMessages_.push(msg);
                                }
                                std::cout << "消息已加入队列" << std::endl;
                            } else {
                                std::cout << "解析消息失败" << std::endl;
                            }
                            // 继续接收下一条消息
                            startReceiving();
                        } else {
                            std::cerr << "接收消息内容失败: " << error.message() << std::endl;
                            isConnected_ = false;
                        }
                    });
            } else {
                std::cerr << "接收消息长度失败: " << error.message() << std::endl;
                isConnected_ = false;
            }
        });
}

void NetworkManager::handleSend(const boost::system::error_code& error,
                              size_t bytes_transferred) {
    if (error) {
        std::cerr << "发送消息失败: " << error.message() << std::endl;
        return;
    }
    
    processMessageQueue(); // 处理队列中的下一条消息
}

void NetworkManager::handleReceive(const boost::system::error_code& error,
                                 size_t bytes_transferred) {
    if (error) {
        std::cerr << "接收消息失败: " << error.message() << std::endl;
        return;
    }
    
    startReceiving(); // 继续接收下一条消息
}

void NetworkManager::processMessageQueue() {
    std::lock_guard<std::mutex> lock(queueMutex_);
    if (messageQueue_.empty()) {
        return;
    }

    try {
        Message msg = messageQueue_.front();
        messageQueue_.pop();

        // 序列化消息
        Json::Value jsonMsg = msg.toJson();
        Json::FastWriter writer;
        std::string message = writer.write(jsonMsg);

        // 发送消息长度
        uint32_t messageLength = message.length();
        boost::asio::write(socket_, boost::asio::buffer(&messageLength, sizeof(messageLength)));

        // 发送消息内容
        boost::asio::write(socket_, boost::asio::buffer(message));
        
        std::cout << "从队列发送消息成功，长度: " << messageLength << std::endl;
    } catch (const boost::system::system_error& e) {
        std::cerr << "从队列发送消息失败: " << e.what() << std::endl;
        isConnected_ = false;
    }
} 