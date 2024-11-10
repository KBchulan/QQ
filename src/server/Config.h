#pragma once

#include <string>
#include <json/json.h>

class Config {
private:
    Json::Value root_;

    Config() {}

public:
    static Config& getInstance() {
        static Config instance;
        return instance;
    }

    bool loadFromFile(const std::string& filename);
    
    std::string getDbHost() const;
    std::string getDbName() const;
    std::string getDbUser() const;
    std::string getDbPassword() const;
    uint16_t getServerPort() const;
    std::string getLogFile() const;
}; 