TARGET=matmul-ijk matmul-kij matmul-jki

CC=gcc
CFLAGS=-g -O3 -Wall -Wextra -std=c99
LDFLAGS=

all: $(TARGET)

matmul-%: mat_mul-%.o timers.o
	$(CC) $(LDFLAGS) $^ -o $@

mat_mul-ijk.o: mat_mul.c
	$(CC) $(CFLAGS) -DIJK -c -o $@ $<
mat_mul-kij.o: mat_mul.c
	$(CC) $(CFLAGS) -DKIJ -c -o $@ $<
mat_mul-jki.o: mat_mul.c
	$(CC) $(CFLAGS) -DJKI -c -o $@ $<

clean:
	rm -f $(TARGET) *.o task_*.stderr task_*.stdout

run: $(TARGET)
	thorq --add ./$(TARGET) -v
