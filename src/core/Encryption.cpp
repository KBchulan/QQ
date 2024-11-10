#include "Encryption.h"
#include <openssl/bio.h>
#include <openssl/buffer.h>
#include <openssl/evp.h>
#include <openssl/rand.h>
#include <stdexcept>
#include <vector>

Encryption::Encryption() : publicKey_(nullptr), privateKey_(nullptr) {
    initAES();
}

Encryption::~Encryption() {
    if (publicKey_) RSA_free(publicKey_);
    if (privateKey_) RSA_free(privateKey_);
}

bool Encryption::generateKeyPair() {
    BIGNUM* bne = BN_new();
    BN_set_word(bne, RSA_F4);
    
    RSA* rsa = RSA_new();
    if (RSA_generate_key_ex(rsa, 2048, bne, nullptr) != 1) {
        BN_free(bne);
        RSA_free(rsa);
        return false;
    }
    
    privateKey_ = rsa;
    publicKey_ = RSAPublicKey_dup(rsa);
    
    BN_free(bne);
    return true;
}

std::string Encryption::encryptPassword(const std::string& password) {
    if (!publicKey_) return "";
    
    std::vector<unsigned char> encrypted(RSA_size(publicKey_));
    int encryptedLength = RSA_public_encrypt(
        password.length(),
        reinterpret_cast<const unsigned char*>(password.c_str()),
        encrypted.data(),
        publicKey_,
        RSA_PKCS1_OAEP_PADDING
    );
    
    if (encryptedLength == -1) return "";
    return base64Encode(encrypted.data(), encryptedLength);
}

std::string Encryption::decryptPassword(const std::string& encryptedPassword) {
    if (!privateKey_) return "";
    
    std::string decoded = base64Decode(encryptedPassword);
    std::vector<unsigned char> decrypted(RSA_size(privateKey_));
    
    int decryptedLength = RSA_private_decrypt(
        decoded.length(),
        reinterpret_cast<const unsigned char*>(decoded.c_str()),
        decrypted.data(),
        privateKey_,
        RSA_PKCS1_OAEP_PADDING
    );
    
    if (decryptedLength == -1) return "";
    return std::string(reinterpret_cast<char*>(decrypted.data()), decryptedLength);
}

void Encryption::initAES() {
    // 生成随机的AES密钥和IV
    if (RAND_bytes(aesKey_, sizeof(aesKey_)) != 1 ||
        RAND_bytes(aesIv_, sizeof(aesIv_)) != 1) {
        throw std::runtime_error("Failed to generate AES key/IV");
    }
}

std::string Encryption::base64Encode(const unsigned char* data, size_t length) {
    BIO* b64 = BIO_new(BIO_f_base64());
    BIO* bmem = BIO_new(BIO_s_mem());
    b64 = BIO_push(b64, bmem);
    BIO_write(b64, data, length);
    BIO_flush(b64);
    
    BUF_MEM* bptr;
    BIO_get_mem_ptr(b64, &bptr);
    std::string result(bptr->data, bptr->length);
    BIO_free_all(b64);
    
    return result;
}

std::string Encryption::base64Decode(const std::string& encoded) {
    BIO* b64 = BIO_new(BIO_f_base64());
    BIO* bmem = BIO_new_mem_buf(encoded.c_str(), encoded.length());
    bmem = BIO_push(b64, bmem);
    
    std::vector<unsigned char> buffer(encoded.length());
    int decodedLength = BIO_read(bmem, buffer.data(), encoded.length());
    BIO_free_all(b64);
    
    return std::string(reinterpret_cast<char*>(buffer.data()), decodedLength);
} 