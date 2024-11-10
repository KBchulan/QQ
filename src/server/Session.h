#pragma once

#include <boost/asio.hpp>
#include <queue>
#include <memory>
#include "../core/Message.h"
#include "Encryption.h"

class Session : public std::enable_shared_from_this<Session> {
public:
    boost::asio::ip::tcp::socket socket_;

private:
    std::queue<Message> messageQueue_;
    std::mutex queueMutex_;
    uint32_t messageLength_;
    std::vector<char> messageBuffer_;
    std::time_t lastHeartbeat_;
    Encryption encryption_;
    bool authenticated_;
    int64_t userId_;

    static constexpr int HEARTBEAT_INTERVAL = 30; // 30秒
    static constexpr int RECONNECT_TIMEOUT = 60; // 60秒

public:
    explicit Session(boost::asio::ip::tcp::socket socket);

    void start();
    void sendMessage(const Message& msg);
    bool isAlive() const;
    int64_t getUserId() const { return userId_; }

private:
    void startRead();
    void handleRead(const boost::system::error_code& error, size_t bytes_transferred);
    void handleWrite(const boost::system::error_code& error);
    void checkHeartbeat();
    void sendHeartbeat();
    void processMessage(const Message& msg);
    void sendRegistrationResponse(bool success, const std::string& error);
    void sendLoginResponse(bool success, const std::string& error, int64_t userId);
}; 