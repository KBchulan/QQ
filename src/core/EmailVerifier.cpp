#include "EmailVerifier.h"
#include <random>
#include <regex>
#include <curl/curl.h>

bool EmailVerifier::sendVerificationCode(const std::string& email) {
    if (!isValidEmail(email)) {
        return false;
    }
    
    // 生成验证码
    std::string code = generateVerificationCode();
    
    // 构造邮件内容
    std::string subject = "验证码";
    std::string body = "您的验证码是: " + code + "\n该验证码将在5分钟后过期。";
    
    // 发送邮件
    if (!sendEmail(email, subject, body)) {
        return false;
    }
    
    // 保存验证信息
    {
        std::lock_guard<std::mutex> lock(mapMutex_);
        VerificationInfo info;
        info.code = code;
        info.expireTime = std::time(nullptr) + CODE_EXPIRE_TIME;
        info.attempts = 0;
        pendingVerifications_[email] = info;
    }
    
    return true;
}

bool EmailVerifier::verifyCode(const std::string& email, const std::string& code) {
    std::lock_guard<std::mutex> lock(mapMutex_);
    
    auto it = pendingVerifications_.find(email);
    if (it == pendingVerifications_.end()) {
        return false;
    }
    
    VerificationInfo& info = it->second;
    
    // 检查是否过期
    if (std::time(nullptr) > info.expireTime) {
        pendingVerifications_.erase(it);
        return false;
    }
    
    // 检查尝试次数
    if (++info.attempts > MAX_ATTEMPTS) {
        pendingVerifications_.erase(it);
        return false;
    }
    
    // 验证码匹配
    if (info.code == code) {
        pendingVerifications_.erase(it);
        return true;
    }
    
    return false;
}

bool EmailVerifier::isValidEmail(const std::string& email) {
    const std::regex pattern(
        R"(^[a-zA-Z0-9._%+-]+@[a-zA-Z0-9.-]+\.[a-zA-Z]{2,}$)"
    );
    return std::regex_match(email, pattern);
}

std::string EmailVerifier::generateVerificationCode() {
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(0, 9);
    
    std::string code;
    for (int i = 0; i < CODE_LENGTH; ++i) {
        code += std::to_string(dis(gen));
    }
    return code;
}

bool EmailVerifier::sendEmail(const std::string& to,
                            const std::string& subject,
                            const std::string& body) {
    // TODO: 实现SMTP邮件发送
    // 这里需要配置SMTP服务器信息
    return true;
}

void EmailVerifier::cleanupExpiredCodes() {
    std::lock_guard<std::mutex> lock(mapMutex_);
    auto now = std::time(nullptr);
    
    for (auto it = pendingVerifications_.begin(); it != pendingVerifications_.end();) {
        if (now > it->second.expireTime) {
            it = pendingVerifications_.erase(it);
        } else {
            ++it;
        }
    }
} 