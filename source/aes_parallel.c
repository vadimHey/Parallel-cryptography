#include <omp.h>
#include <openssl/evp.h>
#include <string.h>
#include <stdlib.h>

void aes_encrypt_parallel(unsigned char* data, size_t len, unsigned char* key, unsigned char* iv, int num_threads, size_t chunk_size) {
    omp_set_num_threads(num_threads);
    
    size_t num_blocks = (len + chunk_size - 1) / chunk_size;
    
    #pragma omp parallel for schedule(dynamic, 1)
    for (size_t block = 0; block < num_blocks; block++) {
        size_t offset = block * chunk_size;
        size_t current_size = (offset + chunk_size <= len) ? chunk_size : (len - offset);
        
        // копия IV
        unsigned char block_iv[16];
        memcpy(block_iv, iv, 16);
        
        // сдвиг счётчика
        for (size_t i = 0; i < block * chunk_size / 16; i++) {
            for (int j = 15; j >= 0; j--) {
                if (++block_iv[j] != 0) break;
            }
        }
        
        EVP_CIPHER_CTX* ctx = EVP_CIPHER_CTX_new();
        EVP_EncryptInit_ex(ctx, EVP_aes_256_ctr(), NULL, key, block_iv);
        
        int out_len;
        EVP_EncryptUpdate(ctx, data + offset, &out_len, data + offset, current_size);
        EVP_EncryptFinal_ex(ctx, data + offset + out_len, &out_len);
        
        EVP_CIPHER_CTX_free(ctx);
    }
}