#include "Config.h"
#include <fstream>
#include <iostream>

bool Config::loadFromFile(const std::string& filename) {
    std::ifstream configFile(filename);
    if (!configFile.is_open()) {
        std::cerr << "无法打开配置文件: " << filename << std::endl;
        return false;
    }

    Json::CharReaderBuilder builder;
    std::string errs;
    if (!Json::parseFromStream(builder, configFile, &root_, &errs)) {
        std::cerr << "解析配置文件失败: " << errs << std::endl;
        return false;
    }

    return true;
}

std::string Config::getDbHost() const {
    return root_["database"]["host"].asString();
}

std::string Config::getDbName() const {
    return root_["database"]["name"].asString();
}

std::string Config::getDbUser() const {
    return root_["database"]["user"].asString();
}

std::string Config::getDbPassword() const {
    std::string password = root_["database"]["password"].asString();
    std::cout << "读取到的数据库密码: " << password << std::endl;
    return password;
}

uint16_t Config::getServerPort() const {
    return root_["server"]["port"].asUInt();
}

std::string Config::getLogFile() const {
    return root_["log"]["file"].asString();
} 