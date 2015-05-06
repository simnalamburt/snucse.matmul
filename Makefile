TARGET=mat_mul_1 mat_mul_2 mat_mul_3 mat_mul_4 mat_mul_5 mat_mul_6 mat_mul_7 mat_mul_8
OBJS=$(patsubst %,%.o,$(TARGET)) timers.o

CC=gcc
CFLAGS=-g -O3 -Wall -Wextra -std=c99
LDFLAGS=-pthread

all: $(TARGET)

mat_mul_%: mat_mul_%.o timers.o
	$(CC) $(LDFLAGS) $^ -o $@


mat_mul_1.o: CFLAGS += -DTHREAD_COUNT=1
mat_mul_2.o: CFLAGS += -DTHREAD_COUNT=2
mat_mul_3.o: CFLAGS += -DTHREAD_COUNT=3
mat_mul_4.o: CFLAGS += -DTHREAD_COUNT=4
mat_mul_5.o: CFLAGS += -DTHREAD_COUNT=5
mat_mul_6.o: CFLAGS += -DTHREAD_COUNT=6
mat_mul_7.o: CFLAGS += -DTHREAD_COUNT=7
mat_mul_8.o: CFLAGS += -DTHREAD_COUNT=8

timers.o: timers.c
	$(CC) $(CFLAGS) -c -o $@ $<
mat_mul_%.o: mat_mul.c
	$(CC) $(CFLAGS) -c -o $@ $<

clean:
	rm -f $(TARGET) *.o task_*.stderr task_*.stdout

run: $(TARGET)
	./mat_mul_1
	./mat_mul_2
	./mat_mul_3
	./mat_mul_4
	./mat_mul_5
	./mat_mul_6
	./mat_mul_7
	./mat_mul_8
