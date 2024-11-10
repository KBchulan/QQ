#include "Server.h"
#include "Logger.h"
#include <iostream>

Server::Server(boost::asio::io_context& io_context, uint16_t port)
    : io_context_(io_context)
    , acceptor_(io_context, boost::asio::ip::tcp::endpoint(boost::asio::ip::tcp::v4(), port)) {
}

void Server::start() {
    try {
        LOG_INFO("服务器启动在端口: " + std::to_string(acceptor_.local_endpoint().port()));
        startAccept();
    }
    catch (const boost::system::system_error& e) {
        if (e.code() == boost::asio::error::address_in_use) {
            LOG_ERROR("端口已被占用，请检查是否有其他服务器实例正在运行");
            throw;
        }
        throw;
    }
}

void Server::startAccept() {
    auto socket = std::make_shared<boost::asio::ip::tcp::socket>(io_context_);
    auto session = std::make_shared<Session>(std::move(*socket));
    
    acceptor_.async_accept(session->socket_,
        [this, session](const boost::system::error_code& error) {
            handleAccept(session, error);
        });
}

void Server::handleAccept(std::shared_ptr<Session> session,
                         const boost::system::error_code& error) {
    if (!error) {
        LOG_INFO("新客户端连接: " + 
                 session->socket_.remote_endpoint().address().to_string() + ":" +
                 std::to_string(session->socket_.remote_endpoint().port()));
        session->start();
        
        // 开始等待下一个连接
        startAccept();
    } else {
        LOG_ERROR("接受连接失败: " + error.message());
    }
} 