#include "Encryption.h"
#include <openssl/rand.h>

Encryption::Encryption() {
    key_.resize(32); // 256位密钥
    iv_.resize(16);  // 128位IV
    RAND_bytes(key_.data(), key_.size());
    RAND_bytes(iv_.data(), iv_.size());
}

std::string Encryption::encrypt(const std::string& data) {
    EVP_CIPHER_CTX *ctx = EVP_CIPHER_CTX_new();
    EVP_EncryptInit_ex(ctx, EVP_aes_256_cbc(), nullptr, key_.data(), iv_.data());
    
    std::vector<unsigned char> encrypted;
    encrypted.resize(data.size() + EVP_MAX_BLOCK_LENGTH);
    int outlen1, outlen2;
    
    EVP_EncryptUpdate(ctx, encrypted.data(), &outlen1,
                      (unsigned char*)data.c_str(), data.size());
    EVP_EncryptFinal_ex(ctx, encrypted.data() + outlen1, &outlen2);
    
    EVP_CIPHER_CTX_free(ctx);
    
    return std::string(encrypted.begin(), encrypted.begin() + outlen1 + outlen2);
}

std::string Encryption::decrypt(const std::string& encryptedData) {
    EVP_CIPHER_CTX *ctx = EVP_CIPHER_CTX_new();
    EVP_DecryptInit_ex(ctx, EVP_aes_256_cbc(), nullptr, key_.data(), iv_.data());
    
    std::vector<unsigned char> decrypted;
    decrypted.resize(encryptedData.size());
    int outlen1, outlen2;
    
    EVP_DecryptUpdate(ctx, decrypted.data(), &outlen1,
                      (unsigned char*)encryptedData.c_str(), encryptedData.size());
    EVP_DecryptFinal_ex(ctx, decrypted.data() + outlen1, &outlen2);
    
    EVP_CIPHER_CTX_free(ctx);
    
    return std::string(decrypted.begin(), decrypted.begin() + outlen1 + outlen2);
} 