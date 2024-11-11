#include "NetworkManager.h"
#include <iostream>
#include <thread>
#include <json/json.h>

bool NetworkManager::connect(const std::string& host, int port) {
    try {
        if (isConnected_) {
            std::cout << "已有连接，断开重连..." << std::endl;
            disconnect();
        }

        shouldStop_ = false;
        socket_ = boost::asio::ip::tcp::socket(io_context_);
        
        boost::asio::ip::tcp::endpoint endpoint(
            boost::asio::ip::address::from_string(host), port);
        
        std::cout << "尝试连接服务器: " << host << ":" << port << std::endl;
        socket_.connect(endpoint);
        
        // 设置socket选项
        boost::asio::socket_base::keep_alive option(true);
        socket_.set_option(option);
        
        isConnected_ = true;
        std::cout << "连接成功，开始接收消息" << std::endl;
        
        startReceiving();
        
        // 启动IO服务线程，捕获endpoint
        std::thread([this, endpoint]() {
            try {
                std::cout << "IO服务线程启动" << std::endl;
                
                // 上次心跳时间
                auto lastHeartbeat = std::chrono::steady_clock::now();
                
                while (!shouldStop_) {
                    try {
                        if (!isConnected_) {
                            std::cout << "检测到连接断开，尝试重新连接..." << std::endl;
                            socket_ = boost::asio::ip::tcp::socket(io_context_);
                            socket_.connect(endpoint);
                            isConnected_ = true;
                            startReceiving();
                        }
                        
                        size_t ops = io_context_.poll();
                        if (ops > 0) {
                            std::cout << "处理了 " << ops << " 个IO操作" << std::endl;
                        }
                        
                        // 每30秒发送一次心跳包
                        auto now = std::chrono::steady_clock::now();
                        if (isConnected_ && 
                            std::chrono::duration_cast<std::chrono::seconds>(now - lastHeartbeat).count() >= 30) {
                            Json::Value heartbeat;
                            heartbeat["type"] = "heartbeat";
                            Message msg(0, 0, heartbeat.toStyledString(), MessageType::HEARTBEAT);
                            sendMessage(msg);
                            lastHeartbeat = now;
                        }
                        
                        std::this_thread::sleep_for(std::chrono::milliseconds(100));
                    } catch (const boost::system::system_error& e) {
                        std::cerr << "IO循环异常: " << e.what() << std::endl;
                        isConnected_ = false;
                        std::this_thread::sleep_for(std::chrono::seconds(1));
                    }
                }
                std::cout << "IO服务线程正常退出" << std::endl;
            } catch (const std::exception& e) {
                std::cerr << "IO服务线程异常退出: " << e.what() << std::endl;
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
    shouldStop_ = true;
    if (isConnected_) {
        try {
            socket_.close();
        } catch (const std::exception& e) {
            std::cerr << "关闭socket异常: " << e.what() << std::endl;
        }
        isConnected_ = false;
    }
    io_context_.stop();
    return true;
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

        // 发送消息长度
        uint32_t messageLength = message.length();
        boost::asio::write(socket_, boost::asio::buffer(&messageLength, sizeof(messageLength)));

        // 发送消息内容
        boost::asio::write(socket_, boost::asio::buffer(message));
        
        if (msg.getType() != MessageType::HEARTBEAT) {  // 不打印心跳包日志
            std::cout << "消息发送成功，长度: " << messageLength << std::endl;
        }
    } catch (const boost::system::system_error& e) {
        std::cerr << "发送消息失败: " << e.what() << std::endl;
        isConnected_ = false;
    }
}

void NetworkManager::startReceiving() {
    if (!isConnected_) {
        std::cout << "未连接，无法开始接收" << std::endl;
        return;
    }

    std::cout << "开始接收消息..." << std::endl;
    
    try {
        auto self = shared_from_this();  // 确保NetworkManager在异步操作期间存活
        boost::asio::async_read(socket_,
            boost::asio::buffer(&messageLength_, sizeof(messageLength_)),
            [this, self](const boost::system::error_code& error, size_t bytes_transferred) {
                if (!error) {
                    std::cout << "收到消息头，长度: " << messageLength_ << std::endl;
                    messageBuffer_.resize(messageLength_);
                    
                    boost::asio::async_read(socket_,
                        boost::asio::buffer(messageBuffer_),
                        [this, self](const boost::system::error_code& error, size_t bytes_transferred) {
                            if (!error) {
                                std::string jsonStr(messageBuffer_.begin(), messageBuffer_.end());
                                std::cout << "收到完整消息: " << jsonStr << std::endl;
                                
                                Json::Value root;
                                Json::Reader reader;
                                if (reader.parse(jsonStr, root)) {
                                    Message msg = Message::fromJson(root);
                                    {
                                        std::lock_guard<std::mutex> lock(receiveMutex_);
                                        receivedMessages_.push(msg);
                                        std::cout << "消息已加入队列" << std::endl;
                                    }
                                }
                                if (isConnected_) {
                                    std::cout << "继续接收下一条消息" << std::endl;
                                    startReceiving();
                                }
                            } else if (error != boost::asio::error::operation_aborted) {
                                std::cerr << "接收消息内容失败: " << error.message() 
                                        << " (" << error.value() << ")" << std::endl;
                                isConnected_ = false;
                            }
                        });
                } else if (error != boost::asio::error::operation_aborted) {
                    std::cerr << "接收消息长度失败: " << error.message() 
                            << " (" << error.value() << ")" << std::endl;
                    isConnected_ = false;
                }
            });
    } catch (const std::exception& e) {
        std::cerr << "接收消息异常: " << e.what() << std::endl;
        isConnected_ = false;
    }
}

bool NetworkManager::hasMessage() {
    std::lock_guard<std::mutex> lock(receiveMutex_);
    return !receivedMessages_.empty();
}

Message NetworkManager::getNextMessage() {
    std::lock_guard<std::mutex> lock(receiveMutex_);
    if (receivedMessages_.empty()) {
        return Message();
    }
    Message msg = receivedMessages_.front();
    receivedMessages_.pop();
    return msg;
} 