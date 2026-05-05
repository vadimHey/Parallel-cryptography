#include <omp.h>
#include <stddef.h>

void xor_encrypt_parallel(unsigned char* data, size_t len, unsigned char* key, size_t key_len, int num_threads, size_t chunk_size) {
    omp_set_num_threads(num_threads);
    
    #pragma omp parallel for schedule(dynamic, chunk_size)
    for (size_t i = 0; i < len; i++) {
        data[i] ^= key[i % key_len];
    }
}