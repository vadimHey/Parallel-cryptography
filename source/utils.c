#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <math.h>

// таймеры
typedef struct timespec timestamp_t;

timestamp_t get_timestamp(void) {
    timestamp_t ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return ts;
}

double time_diff_seconds(timestamp_t start, timestamp_t end) {
    return (end.tv_sec - start.tv_sec) + (end.tv_nsec - start.tv_nsec) / 1e9;
}

// работа с файлами
int load_file(const char* filename, unsigned char** data, size_t* size) {
    FILE* f = fopen(filename, "rb");
    if (!f) return -1;
    
    fseek(f, 0, SEEK_END);
    *size = ftell(f);
    fseek(f, 0, SEEK_SET);
    
    *data = malloc(*size);
    if (!*data) {
        fclose(f);
        return -1;
    }
    
    size_t read_bytes = fread(*data, 1, *size, f);
    fclose(f);
    
    return (read_bytes == *size) ? 0 : -1;
}

void save_file(const char* filename, unsigned char* data, size_t size) {
    FILE* f = fopen(filename, "wb");
    if (f) {
        fwrite(data, 1, size, f);
        fclose(f);
    }
}

// статистика
double mean(double* values, int n) {
    double sum = 0;
    for (int i = 0; i < n; i++) sum += values[i];
    return sum / n;
}