#pragma once

#include <string>
#include <vector>
#include <openssl/aes.h>
#include <openssl/evp.h>

class Encryption {
private:
    std::vector<unsigned char> key_;
    std::vector<unsigned char> iv_;

public:
    Encryption();
    std::string encrypt(const std::string& data);
    std::string decrypt(const std::string& encryptedData);
    std::string generateKey();
}; 