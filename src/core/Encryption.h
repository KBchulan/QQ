#pragma once
#include <string>
#include <openssl/rsa.h>
#include <openssl/pem.h>
#include <openssl/aes.h>
#include <openssl/evp.h>

class Encryption {
private:
    RSA* publicKey_;
    RSA* privateKey_;
    unsigned char aesKey_[32];  // AES-256 密钥
    unsigned char aesIv_[16];   // AES IV

public:
    Encryption();
    ~Encryption();

    // RSA密钥对生成和管理
    bool generateKeyPair();
    bool loadPublicKey(const std::string& path);
    bool loadPrivateKey(const std::string& path);
    std::string getPublicKeyString() const;

    // 密码加密
    std::string encryptPassword(const std::string& password);
    std::string decryptPassword(const std::string& encryptedPassword);

    // AES加密（用于消息传输）
    std::string encryptMessage(const std::string& message);
    std::string decryptMessage(const std::string& encryptedMessage);

private:
    void initAES();
    std::string base64Encode(const unsigned char* data, size_t length);
    std::string base64Decode(const std::string& encoded);
}; 