TARGET=mat_mul_1 mat_mul_2 mat_mul_3 mat_mul_4 mat_mul_5 mat_mul_6 mat_mul_7 mat_mul_8 mat_mul_16 mat_mul_32 mat_mul_64 mat_mul_128 mat_mul_256 mat_mul_512 mat_mul_1024 mat_mul_2048
OBJS=$(patsubst %,%.o,$(TARGET)) timers.o

CC=gcc
CFLAGS=-g -O3 -Wall -Wextra -std=c99
LDFLAGS=

all: $(TARGET)

mat_mul_%: mat_mul_%.o timers.o
	$(CC) $(LDFLAGS) $^ -o $@

mat_mul_1.o: CFLAGS += -DB=1
mat_mul_2.o: CFLAGS += -DB=2
mat_mul_3.o: CFLAGS += -DB=3
mat_mul_4.o: CFLAGS += -DB=4
mat_mul_5.o: CFLAGS += -DB=5
mat_mul_6.o: CFLAGS += -DB=6
mat_mul_7.o: CFLAGS += -DB=7
mat_mul_8.o: CFLAGS += -DB=8
mat_mul_16.o: CFLAGS += -DB=16
mat_mul_32.o: CFLAGS += -DB=32
mat_mul_64.o: CFLAGS += -DB=64
mat_mul_128.o: CFLAGS += -DB=128
mat_mul_256.o: CFLAGS += -DB=256
mat_mul_512.o: CFLAGS += -DB=512
mat_mul_1024.o: CFLAGS += -DB=1024
mat_mul_2048.o: CFLAGS += -DB=2048

timers.o: timers.c
	$(CC) $(CFLAGS) -c -o $@ $<
mat_mul_%.o: mat_mul.c
	$(CC) $(CFLAGS) -c -o $@ $<

clean:
	rm -f $(TARGET) *.o task_*.stderr task_*.stdout

run: $(TARGET)
	thorq --add ./mat_mul_1
	thorq --add ./mat_mul_2
	thorq --add ./mat_mul_3
	thorq --add ./mat_mul_4
	thorq --add ./mat_mul_5
	thorq --add ./mat_mul_6
	thorq --add ./mat_mul_7
	thorq --add ./mat_mul_8
	thorq --add ./mat_mul_16
	thorq --add ./mat_mul_32
	thorq --add ./mat_mul_64
	thorq --add ./mat_mul_128
	thorq --add ./mat_mul_256
	thorq --add ./mat_mul_512
	thorq --add ./mat_mul_1024
	thorq --add ./mat_mul_2048
