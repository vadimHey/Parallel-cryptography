#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <getopt.h>

typedef struct timespec timestamp_t;
timestamp_t get_timestamp(void);
double time_diff_seconds(timestamp_t start, timestamp_t end);
int load_file(const char* filename, unsigned char** data, size_t* size);
void save_file(const char* filename, unsigned char* data, size_t size);
double mean(double* values, int n);
double confidence_interval_95(double* values, int n);
void xor_encrypt(unsigned char* data, size_t len, unsigned char* key, size_t key_len);
void xor_encrypt_parallel(unsigned char* data, size_t len, unsigned char* key, size_t key_len, int num_threads, size_t chunk_size);
void aes_encrypt(unsigned char* data, size_t len, unsigned char* key, unsigned char* iv);
void aes_encrypt_parallel(unsigned char* data, size_t len, unsigned char* key, unsigned char* iv, int num_threads, size_t chunk_size);

static unsigned char aes_key[32];
static unsigned char aes_iv[16];
static unsigned char xor_key[16];

void init_keys(void) {
    memset(aes_key, 0x42, 32);
    memset(aes_iv, 0x24, 16);
    memset(xor_key, 0x5A, 16);
}

// вывод опций использования 
void print_usage(const char* prog) {
    printf("Usage: %s [options]\n", prog);
    printf("  -a, --algo     aes or xor\n");
    printf("  -f, --file     input file\n");
    printf("  -p, --parallel use OpenMP\n");
    printf("  -t, --threads  N threads\n");
    printf("  -c, --chunk    chunk size in bytes\n");
    printf("  -m, --meas     number of measurements\n");
    printf("  -o, --output   output CSV file\n");
    printf("  -h, --help     help\n");
}

int main(int argc, char* argv[]) {
    char* algo = "xor";
    char* filename = NULL;
    int parallel = 0;
    int num_threads = 1;
    size_t chunk_size = 65536;
    int num_measurements = 30;
    char* output_file = NULL;
    
    // парсинг аргументов 
    static struct option long_opts[] = {
        {"algo", required_argument, 0, 'a'},
        {"file", required_argument, 0, 'f'},
        {"parallel", no_argument, 0, 'p'},
        {"threads", required_argument, 0, 't'},
        {"chunk", required_argument, 0, 'c'},
        {"meas", required_argument, 0, 'm'},
        {"output", required_argument, 0, 'o'},
        {"help", no_argument, 0, 'h'},
        {0,0,0,0}
    };
    
    int opt;
    while ((opt = getopt_long(argc, argv, "a:f:pt:c:m:o:h", long_opts, NULL)) != -1) {
        switch (opt) {
            case 'a': algo = optarg; break;
            case 'f': filename = optarg; break;
            case 'p': parallel = 1; break;
            case 't': num_threads = atoi(optarg); break;
            case 'c': chunk_size = atol(optarg); break;
            case 'm': num_measurements = atoi(optarg); break;
            case 'o': output_file = optarg; break;
            case 'h': print_usage(argv[0]); return 0;
            default: return 1;
        }
    }
    
    if (!filename) {
        fprintf(stderr, "Error: input file required\n");
        return 1;
    }
    
    init_keys();
    
    unsigned char* data;
    size_t data_size;
    if (load_file(filename, &data, &data_size) != 0) {
        fprintf(stderr, "Failed to load file\n");
        return 1;
    }
    
    printf("File: %s (%zu bytes)\n", filename, data_size);
    printf("Algorithm: %s, Mode: %s\n", algo, parallel ? "Parallel" : "Sequential");
    
    double* times = malloc(num_measurements * sizeof(double));
    
    for (int i = 0; i < num_measurements; i++) {
        unsigned char* copy = malloc(data_size);
        memcpy(copy, data, data_size);
        
        timestamp_t start = get_timestamp();
        
        if (parallel) {
            if (strcmp(algo, "aes") == 0) {
                aes_encrypt_parallel(copy, data_size, aes_key, aes_iv, num_threads, chunk_size);
            } else {
                xor_encrypt_parallel(copy, data_size, xor_key, 16, num_threads, chunk_size);
            }
        } else {
            if (strcmp(algo, "aes") == 0) {
                aes_encrypt(copy, data_size, aes_key, aes_iv);
            } else {
                xor_encrypt(copy, data_size, xor_key, 16);
            }
        }
        
        timestamp_t end = get_timestamp();
        times[i] = time_diff_seconds(start, end);
        
        free(copy);
        
        if ((i + 1) % 10 == 0) {
            printf("Progress: %d/%d\n", i + 1, num_measurements);
        }
    }
    
    double avg = mean(times, num_measurements);
    
    printf("\nResults: %.6f sec\n", avg);
    
    if (output_file) {
        FILE* out = fopen(output_file, "w");
        if (out) {
            fprintf(out, "measurement,time_seconds\n");
            for (int i = 0; i < num_measurements; i++) {
                fprintf(out, "%d,%.6f\n", i + 1, times[i]);
            }
            fclose(out);
            printf("Saved to: %s\n", output_file);
        }
    }
    
    free(times);
    free(data);
    return 0;
}