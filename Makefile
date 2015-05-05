TARGET=matmul-ijk matmul-ikj matmul-kij matmul-kji matmul-jki matmul-jik

CC=gcc
CFLAGS=-g -O3 -Wall -Wextra -std=c99
LDFLAGS=

all: $(TARGET)

matmul-%: mat_mul-%.o timers.o
	$(CC) $(LDFLAGS) $^ -o $@

mat_mul-ijk.o: mat_mul.c
	$(CC) $(CFLAGS) -DIJK -c -o $@ $<
mat_mul-ikj.o: mat_mul.c
	$(CC) $(CFLAGS) -DIKJ -c -o $@ $<
mat_mul-kij.o: mat_mul.c
	$(CC) $(CFLAGS) -DKIJ -c -o $@ $<
mat_mul-kji.o: mat_mul.c
	$(CC) $(CFLAGS) -DKJI -c -o $@ $<
mat_mul-jki.o: mat_mul.c
	$(CC) $(CFLAGS) -DJKI -c -o $@ $<
mat_mul-jik.o: mat_mul.c
	$(CC) $(CFLAGS) -DJIK -c -o $@ $<

clean:
	rm -f $(TARGET) *.o task_*.stderr task_*.stdout

run: $(TARGET)
	./matmul-ikj
	./matmul-kij
	./matmul-ijk
	./matmul-jik
	./matmul-kji
	./matmul-jki
