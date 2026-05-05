#include <stddef.h>

void xor_encrypt(unsigned char* data, size_t len, unsigned char* key, size_t key_len) {
    for (size_t i = 0; i < len; i++) {
        data[i] ^= key[i % key_len];
    }
}