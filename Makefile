TARGET=mat_mul_1 mat_mul_4 mat_mul_9 mat_mul_16
OBJS=$(patsubst %,%.o,$(TARGET)) timers.o

CC=gcc
CFLAGS=-g -O3 -Wall -Wextra -std=c99
LDFLAGS=-pthread

all: $(TARGET)

mat_mul_%: mat_mul_%.o timers.o
	$(CC) $(LDFLAGS) $^ -o $@


mat_mul_1.o: CFLAGS += -DJOB_COUNT=1
mat_mul_4.o: CFLAGS += -DJOB_COUNT=2
mat_mul_9.o: CFLAGS += -DJOB_COUNT=3
mat_mul_16.o: CFLAGS += -DJOB_COUNT=4

timers.o: timers.c
	$(CC) $(CFLAGS) -c -o $@ $<
mat_mul_%.o: mat_mul.c
	$(CC) $(CFLAGS) -c -o $@ $<

clean:
	rm -f $(TARGET) *.o task_*.stderr task_*.stdout

run: $(TARGET)
	./mat_mul_1
	./mat_mul_4
	./mat_mul_9
	./mat_mul_16
