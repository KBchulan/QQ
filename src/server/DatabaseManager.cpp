#include "DatabaseManager.h"
#include "Logger.h"
#include <sstream>
#include <iostream>
#include <iomanip>
#include <openssl/evp.h>

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
                                     const std::string& password,
                                     int64_t& userId) {
    LOG_INFO("验证用户登录 - 用户名: " + username);
    
    // 对密码进行哈希
    std::string passwordHash = hashPassword(password);
    LOG_INFO("密码哈希值: " + passwordHash);

    // 转义用户名以防SQL注入
    char escaped_username[username.length() * 2 + 1];
    mysql_real_escape_string(conn_, escaped_username, username.c_str(), username.length());

    std::stringstream ss;
    ss << "SELECT user_id, password_hash FROM users WHERE username = '" 
       << escaped_username << "'";
    
    LOG_INFO("执行SQL: " + ss.str());
    
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
        std::string storedHash = row[1];
        LOG_INFO("数据库中的密码哈希: " + storedHash);
        
        if (storedHash == passwordHash) {
            userId = std::stoll(row[0]);
            mysql_free_result(result);
            LOG_INFO("密码验证成功，用户ID: " + std::to_string(userId));
            return true;
        } else {
            LOG_WARNING("密码不匹配");
        }
    } else {
        LOG_WARNING("用户名不存在: " + username);
    }

    mysql_free_result(result);
    return false;
}

std::string DatabaseManager::hashPassword(const std::string& password) {
    unsigned char hash[EVP_MAX_MD_SIZE];
    unsigned int hashLen;

    // 使用新的EVP接口
    EVP_MD_CTX* ctx = EVP_MD_CTX_new();
    if (!ctx) {
        LOG_ERROR("创建EVP上下文失败");
        return "";
    }

    if (EVP_DigestInit_ex(ctx, EVP_sha256(), nullptr) != 1) {
        LOG_ERROR("初始化SHA256失败");
        EVP_MD_CTX_free(ctx);
        return "";
    }

    if (EVP_DigestUpdate(ctx, password.c_str(), password.length()) != 1) {
        LOG_ERROR("更新哈希数据失败");
        EVP_MD_CTX_free(ctx);
        return "";
    }

    if (EVP_DigestFinal_ex(ctx, hash, &hashLen) != 1) {
        LOG_ERROR("生成最终哈希失败");
        EVP_MD_CTX_free(ctx);
        return "";
    }

    EVP_MD_CTX_free(ctx);

    // 转换为十六进制字符串
    std::stringstream ss;
    for(unsigned int i = 0; i < hashLen; i++) {
        ss << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(hash[i]);
    }
    
    std::string result = ss.str();
    LOG_INFO("生成密码哈希: " + result);
    return result;
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