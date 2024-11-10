#include "UserManager.h"
#include "Logger.h"
#include <openssl/evp.h>
#include <sstream>
#include <iomanip>

std::string UserManager::hashPassword(const std::string& password) {
    unsigned char hash[EVP_MAX_MD_SIZE];
    unsigned int hashLen;

    // 使用新的EVP接口
    EVP_MD_CTX* ctx = EVP_MD_CTX_new();
    EVP_DigestInit_ex(ctx, EVP_sha256(), nullptr);
    EVP_DigestUpdate(ctx, password.c_str(), password.length());
    EVP_DigestFinal_ex(ctx, hash, &hashLen);
    EVP_MD_CTX_free(ctx);

    // 转换为十六进制字符串
    std::stringstream ss;
    for(unsigned int i = 0; i < hashLen; i++) {
        ss << std::hex << std::setw(2) << std::setfill('0') << (int)hash[i];
    }
    return ss.str();
}

bool UserManager::registerUser(const std::string& username,
                             const std::string& password,
                             const std::string& nickname) {
    LOG_INFO("开始注册用户: " + username);
    
    if (username.empty() || password.empty()) {
        LOG_ERROR("用户名或密码为空");
        return false;
    }
    
    // 开始事务
    if (!DatabaseManager::getInstance().executeQuery("START TRANSACTION")) {
        LOG_ERROR("开始事务失败");
        return false;
    }

    try {
        // 检查用户名是否已存在
        std::string checkQuery = "SELECT COUNT(*) FROM users WHERE username = '" + username + "'";
        MYSQL_RES* result = DatabaseManager::getInstance().executeQueryWithResult(checkQuery);
        if (!result) {
            LOG_ERROR("检查用户名失败");
            DatabaseManager::getInstance().executeQuery("ROLLBACK");
            return false;
        }

        MYSQL_ROW row = mysql_fetch_row(result);
        if (row && std::stoi(row[0]) > 0) {
            LOG_WARNING("用户名已存在: " + username);
            mysql_free_result(result);
            DatabaseManager::getInstance().executeQuery("ROLLBACK");
            return false;
        }
        mysql_free_result(result);

        // 对密码进行哈希处理
        std::string passwordHash = hashPassword(password);
        LOG_INFO("密码哈希值生成成功");

        // 构造插入SQL
        std::stringstream ss;
        ss << "INSERT INTO users (username, password_hash, nickname, created_at) VALUES ("
           << "'" << username << "', "
           << "'" << passwordHash << "', "
           << "'" << (nickname.empty() ? username : nickname) << "', "
           << "NOW())";

        LOG_INFO("执行注册SQL: " + ss.str());

        // 执行插入
        if (!DatabaseManager::getInstance().executeQuery(ss.str())) {
            LOG_ERROR("执行插入失败");
            DatabaseManager::getInstance().executeQuery("ROLLBACK");
            return false;
        }

        // 提交事务
        if (!DatabaseManager::getInstance().executeQuery("COMMIT")) {
            LOG_ERROR("提交事务失败");
            DatabaseManager::getInstance().executeQuery("ROLLBACK");
            return false;
        }

        LOG_INFO("用户注册成功: " + username);
        return true;
    } catch (const std::exception& e) {
        LOG_ERROR("注册过程发生异常: " + std::string(e.what()));
        DatabaseManager::getInstance().executeQuery("ROLLBACK");
        return false;
    }
}

bool UserManager::login(const std::string& username,
                       const std::string& password,
                       int64_t& userId) {
    LOG_INFO("尝试登录用户: " + username);
    
    // 验证用户名和密码
    std::string passwordHash = hashPassword(password);
    if (!DatabaseManager::getInstance().authenticateUser(username, passwordHash, userId)) {
        LOG_WARNING("用户登录失败：用户名或密码错误");
        return false;
    }

    // 更新用户状态
    {
        std::lock_guard<std::mutex> lock(usersMutex_);
        auto user = std::make_shared<User>();
        user->setUserId(userId);
        user->setUsername(username);
        user->setOnline(true);
        user->setLastLoginTime(std::time(nullptr));
        onlineUsers_[userId] = user;
    }

    // 更新数据库中的用户状态
    DatabaseManager::getInstance().updateUserStatus(userId, true);

    LOG_INFO("用户 " + username + " 登录成功");
    return true;
}

std::shared_ptr<User> UserManager::getUserByUsername(const std::string& username) {
    std::stringstream ss;
    ss << "SELECT user_id, username, nickname, avatar_url, status, last_login "
       << "FROM users WHERE username = '" << username << "'";

    MYSQL_RES* result = DatabaseManager::getInstance().executeQueryWithResult(ss.str());
    if (!result) {
        LOG_ERROR("查询用户失败: " + DatabaseManager::getInstance().getLastError());
        return nullptr;
    }

    MYSQL_ROW row = mysql_fetch_row(result);
    if (row) {
        auto user = std::make_shared<User>();
        user->setUserId(std::stoll(row[0]));
        user->setUsername(row[1]);
        user->setNickname(row[2] ? row[2] : "");
        user->setAvatarUrl(row[3] ? row[3] : "");
        user->setOnline(std::stoi(row[4]) != 0);
        mysql_free_result(result);
        return user;
    }

    mysql_free_result(result);
    return nullptr;
}