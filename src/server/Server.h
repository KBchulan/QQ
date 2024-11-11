#pragma once

#include <boost/asio.hpp>
#include <unordered_map>
#include <memory>
#include "Session.h"
#include "../core/Message.h"

class Server {
private:
    boost::asio::io_context& io_context_;
    boost::asio::ip::tcp::acceptor acceptor_;
    std::unordered_map<int64_t, std::shared_ptr<Session>> sessions_;
    std::mutex sessionsMutex_;
    
    static Server* instance_;

public:
    Server(boost::asio::io_context& io_context, uint16_t port);
    
    void start();
    
    static Server& getInstance() {
        if (!instance_) {
            throw std::runtime_error("Server instance not initialized");
        }
        return *instance_;
    }
    
    std::shared_ptr<Session> getSession(int64_t userId) {
        std::lock_guard<std::mutex> lock(sessionsMutex_);
        auto it = sessions_.find(userId);
        return (it != sessions_.end()) ? it->second : nullptr;
    }

    void broadcastMessage(const Message& msg);
    void removeSession(int64_t userId);

private:
    void startAccept();
    void handleAccept(std::shared_ptr<Session> session,
                     const boost::system::error_code& error);
}; 