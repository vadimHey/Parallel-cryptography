CC = gcc
CFLAGS = -Wall -O2 -march=native -fopenmp
LDFLAGS = -lm -lssl -lcrypto

SRC = src/main.c \
      src/utils.c \
      src/xor_sequential.c \
      src/xor_parallel.c \
      src/aes_sequential.c \
      src/aes_parallel.c

TARGET = bin/parallel_crypto

all:
	@mkdir -p bin
	$(CC) $(CFLAGS) $(SRC) -o $(TARGET) $(LDFLAGS)

test: all
	@mkdir -p data
	dd if=/dev/urandom of=data/test.bin bs=1M count=1 2>/dev/null
	./$(TARGET) -f data/test.bin -a xor
	./$(TARGET) -f data/test.bin -a xor -p -t 4

clean:
	rm -rf bin data results

generate:
	@mkdir -p data
	dd if=/dev/urandom of=data/1MB.bin bs=1M count=1
	dd if=/dev/urandom of=data/5MB.bin bs=1M count=5
	dd if=/dev/urandom of=data/10MB.bin bs=1M count=10
	dd if=/dev/urandom of=data/25MB.bin bs=1M count=25
	dd if=/dev/urandom of=data/50MB.bin bs=1M count=50

help:
	@echo "make          - build"
	@echo "make test     - quick test"
	@echo "make generate - create test files (1-50 MB)"
	@echo "make clean    - clean"