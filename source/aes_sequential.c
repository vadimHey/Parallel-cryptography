#include <openssl/evp.h>
#include <string.h>

void aes_encrypt(unsigned char* data, size_t len, unsigned char* key, unsigned char* iv) {
    EVP_CIPHER_CTX* ctx = EVP_CIPHER_CTX_new();
    EVP_EncryptInit_ex(ctx, EVP_aes_256_ctr(), NULL, key, iv);
    
    int out_len;
    EVP_EncryptUpdate(ctx, data, &out_len, data, len);
    EVP_EncryptFinal_ex(ctx, data + out_len, &out_len);
    
    EVP_CIPHER_CTX_free(ctx);
}