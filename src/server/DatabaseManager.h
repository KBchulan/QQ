#pragma once

#ifdef __linux__
    #include <mysql/mysql.h>
#else
    #include <mysql.h>
#endif

#include <string>
#include <memory>
#include <vector>
#include "../core/Message.h"

class DatabaseManager {
private:
    MYSQL* conn_;
    static DatabaseManager instance_;

    std::string host_;
    std::string database_;
    std::string user_;
    std::string password_;

    DatabaseManager() : conn_(nullptr) {}
    ~DatabaseManager();

public:
    static DatabaseManager& getInstance() {
        static DatabaseManager instance;
        return instance;
    }

    bool initialize(const std::string& host, 
                   const std::string& database,
                   const std::string& user,
                   const std::string& password);

    bool storeMessage(const Message& msg);
    bool authenticateUser(const std::string& username, 
                         const std::string& password,
                         int64_t& userId);
    bool updateUserStatus(int64_t userId, bool online);
    std::vector<Message> getOfflineMessages(int64_t userId);
    bool executeQuery(const std::string& query);

    MYSQL_RES* executeQueryWithResult(const std::string& query);
    std::string getLastError() const { return mysql_error(conn_); }

    MYSQL* getConnection() { return conn_; }
}; 