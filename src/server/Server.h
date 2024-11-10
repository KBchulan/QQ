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

public:
    Server(boost::asio::io_context& io_context, uint16_t port);
    void start();

private:
    void startAccept();
    void handleAccept(std::shared_ptr<Session> session,
                     const boost::system::error_code& error);
}; 