#pragma once

#include <boost/asio.hpp>
#include <string>
#include <queue>
#include <mutex>
#include <memory>
#include "Message.h"

class NetworkManager : public std::enable_shared_from_this<NetworkManager> {
private:
    boost::asio::io_context io_context_;
    boost::asio::ip::tcp::socket socket_;
    std::queue<Message> messageQueue_;
    std::mutex queueMutex_;
    bool isConnected_;
    std::queue<Message> receivedMessages_;
    std::mutex receiveMutex_;
    
    uint32_t messageLength_;
    std::vector<char> messageBuffer_;

public:
    NetworkManager();
    ~NetworkManager();

    bool connect(const std::string& host, int port);
    bool disconnect();
    bool isConnected() const { return isConnected_; }
    
    void sendMessage(const Message& msg);
    void startReceiving();

    bool hasMessage() {
        std::lock_guard<std::mutex> lock(receiveMutex_);
        return !receivedMessages_.empty();
    }

    Message getNextMessage() {
        std::lock_guard<std::mutex> lock(receiveMutex_);
        if (receivedMessages_.empty()) {
            return Message();
        }
        Message msg = receivedMessages_.front();
        receivedMessages_.pop();
        return msg;
    }

private:
    void handleSend(const boost::system::error_code& error, size_t bytes_transferred);
    void handleReceive(const boost::system::error_code& error, size_t bytes_transferred);
    void processMessageQueue();
}; 