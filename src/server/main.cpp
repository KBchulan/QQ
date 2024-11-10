#include <iostream>
#include <boost/asio.hpp>
#include "Server.h"
#include "DatabaseManager.h"
#include "Config.h"
#include "Logger.h"

int main(int argc, char* argv[]) {
    try {
        // 加载配置文件
        if (!Config::getInstance().loadFromFile("config/server_config.json")) {
            std::cerr << "无法加载配置文件" << std::endl;
            return 1;
        }

        // 设置日志文件
        Logger::getInstance().setLogFile(Config::getInstance().getLogFile());
        LOG_INFO("服务器启动中...");

        // 打印数据库配置信息
        LOG_INFO("数据库配置:");
        LOG_INFO("Host: " + Config::getInstance().getDbHost());
        LOG_INFO("Database: " + Config::getInstance().getDbName());
        LOG_INFO("User: " + Config::getInstance().getDbUser());

        // 初始化数据库连接
        if (!DatabaseManager::getInstance().initialize(
                Config::getInstance().getDbHost(),
                Config::getInstance().getDbName(),
                Config::getInstance().getDbUser(),
                Config::getInstance().getDbPassword())) {
            LOG_ERROR("数据库初始化失败");
            return 1;
        }
        LOG_INFO("数据库连接成功");

        // 创建IO上下文和服务器
        boost::asio::io_context io_context;
        Server server(io_context, Config::getInstance().getServerPort());
        
        LOG_INFO("服务器启动成功，监听端口: " + 
                 std::to_string(Config::getInstance().getServerPort()));
        
        // 启动服务器
        server.start();
        
        // 运行IO服务
        io_context.run();
    }
    catch (std::exception& e) {
        LOG_ERROR("服务器错误: " + std::string(e.what()));
        return 1;
    }

    return 0;
} 