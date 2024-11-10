#include "DatabaseManager.h"
#include "Logger.h"
#include <sstream>
#include <iostream>

DatabaseManager::~DatabaseManager() {
    if (conn_) {
        mysql_close(conn_);
    }
}

bool DatabaseManager::initialize(const std::string& host,
                               const std::string& database,
                               const std::string& user,
                               const std::string& password) {
    host_ = host;
    database_ = database;
    user_ = user;
    password_ = password;

    conn_ = mysql_init(nullptr);
    if (!conn_) {
        LOG_ERROR("MySQL初始化失败");
        return false;
    }

    if (!mysql_real_connect(conn_, host.c_str(), user.c_str(),
                           password.c_str(), database.c_str(), 0, nullptr, 0)) {
        LOG_ERROR("数据库连接失败: " + std::string(mysql_error(conn_)));
        return false;
    }

    LOG_INFO("数据库连接成功");
    return true;
}

bool DatabaseManager::storeMessage(const Message& msg) {
    std::stringstream ss;
    ss << "INSERT INTO messages (sender_id, receiver_id, content, msg_type, send_time) "
       << "VALUES (" << msg.getSenderId() << ", " << msg.getReceiverId() << ", '"
       << msg.getContent() << "', " << static_cast<int>(msg.getType()) << ", NOW())";

    return executeQuery(ss.str());
}

bool DatabaseManager::authenticateUser(const std::string& username,
                                     const std::string& password_hash,
                                     int64_t& userId) {
    std::stringstream ss;
    ss << "SELECT user_id, username FROM users WHERE "
       << "username='" << username << "' AND "
       << "password_hash='" << password_hash << "'";

    LOG_INFO("执行登录验证SQL: " + ss.str());

    MYSQL_RES* result = nullptr;
    if (mysql_query(conn_, ss.str().c_str()) != 0) {
        LOG_ERROR("认证查询失败: " + std::string(mysql_error(conn_)));
        return false;
    }

    result = mysql_store_result(conn_);
    if (!result) {
        LOG_ERROR("获取认证结果失败: " + std::string(mysql_error(conn_)));
        return false;
    }

    MYSQL_ROW row = mysql_fetch_row(result);
    if (row) {
        userId = std::stoll(row[0]);
        mysql_free_result(result);
        
        // 更新用户状态和最后登录时间
        std::stringstream updateSs;
        updateSs << "UPDATE users SET status = 1, last_login = NOW() "
                << "WHERE user_id = " << userId;
        if (mysql_query(conn_, updateSs.str().c_str()) != 0) {
            LOG_WARNING("更新用户状态失败: " + std::string(mysql_error(conn_)));
        }
        
        LOG_INFO("用户 " + username + " (ID: " + std::to_string(userId) + ") 认证成功");
        return true;
    }

    mysql_free_result(result);
    LOG_WARNING("用户 " + username + " 认证失败");
    return false;
}

bool DatabaseManager::executeQuery(const std::string& query) {
    LOG_INFO("执行SQL: " + query);
    
    if (!conn_) {
        LOG_ERROR("数据库连接初始化");
        return false;
    }
    
    if (mysql_ping(conn_) != 0) {
        LOG_ERROR("数据库连接已断开，尝试重连");
        if (mysql_real_connect(conn_, host_.c_str(), user_.c_str(),
                             password_.c_str(), database_.c_str(), 0, nullptr, 0) == nullptr) {
            LOG_ERROR("数据库重连失败: " + std::string(mysql_error(conn_)));
            return false;
        }
    }
    
    if (mysql_query(conn_, query.c_str()) != 0) {
        LOG_ERROR("执行查询失败: " + std::string(mysql_error(conn_)) + "\nSQL: " + query);
        return false;
    }
    
    LOG_INFO("SQL执行成功");
    return true;
}

bool DatabaseManager::updateUserStatus(int64_t userId, bool online) {
    std::stringstream ss;
    ss << "UPDATE users SET status = " << (online ? 1 : 0)
       << " WHERE user_id = " << userId;
    
    if (executeQuery(ss.str())) {
        LOG_INFO("用户 " + std::to_string(userId) + " 状态更新为: " + 
                 (online ? "在线" : "离线"));
        return true;
    }
    return false;
}

std::vector<Message> DatabaseManager::getOfflineMessages(int64_t userId) {
    std::vector<Message> messages;
    std::stringstream ss;
    
    ss << "SELECT msg_id, sender_id, receiver_id, content, msg_type, send_time "
       << "FROM messages WHERE receiver_id = " << userId
       << " AND status = 0 ORDER BY send_time ASC";

    if (mysql_query(conn_, ss.str().c_str()) != 0) {
        LOG_ERROR("获取离线消息失败: " + std::string(mysql_error(conn_)));
        return messages;
    }

    MYSQL_RES* result = mysql_store_result(conn_);
    if (!result) {
        LOG_ERROR("存储结果集失败: " + std::string(mysql_error(conn_)));
        return messages;
    }

    MYSQL_ROW row;
    int messageCount = 0;
    while ((row = mysql_fetch_row(result))) {
        // 构造Message对象并添加到vector中
        Message msg(
            std::stoll(row[1]),  // sender_id
            std::stoll(row[2]),  // receiver_id
            row[3],              // content
            static_cast<MessageType>(std::stoi(row[4]))  // msg_type
        );
        messages.push_back(msg);
        messageCount++;
    }

    mysql_free_result(result);
    LOG_INFO("获取到 " + std::to_string(messageCount) + " 条离线消息");
    return messages;
}

MYSQL_RES* DatabaseManager::executeQueryWithResult(const std::string& query) {
    if (mysql_query(conn_, query.c_str()) != 0) {
        LOG_ERROR("执行查询失败: " + std::string(mysql_error(conn_)));
        return nullptr;
    }
    
    MYSQL_RES* result = mysql_store_result(conn_);
    if (!result) {
        LOG_ERROR("存储结果集失败: " + std::string(mysql_error(conn_)));
        return nullptr;
    }
    
    return result;
} 