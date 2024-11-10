#pragma once
#include <string>
#include <ctime>
#include <unordered_map>
#include <mutex>

class EmailVerifier {
private:
    struct VerificationInfo {
        std::string code;
        std::time_t expireTime;
        int attempts;
    };

    std::unordered_map<std::string, VerificationInfo> pendingVerifications_;
    std::mutex mapMutex_;
    static const int CODE_LENGTH = 6;
    static const int CODE_EXPIRE_TIME = 300;  // 5分钟
    static const int MAX_ATTEMPTS = 3;

public:
    static EmailVerifier& getInstance() {
        static EmailVerifier instance;
        return instance;
    }

    // 发送验证码
    bool sendVerificationCode(const std::string& email);
    
    // 验证码验证
    bool verifyCode(const std::string& email, const std::string& code);
    
    // 检查邮箱格式
    static bool isValidEmail(const std::string& email);

private:
    EmailVerifier() {}
    std::string generateVerificationCode();
    bool sendEmail(const std::string& to, const std::string& subject, const std::string& body);
    void cleanupExpiredCodes();
}; 