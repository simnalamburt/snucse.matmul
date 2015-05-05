TARGET=mat_mul
OBJS=mat_mul.o timers.o

CC=gcc
CFLAGS=-g -O3 -Wall -Wextra -std=c99
LDFLAGS=

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(LDFLAGS) $(OBJS) -o $@

%.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $<

clean:
	rm -f $(TARGET) *.o task_*.stderr task_*.stdout

run: $(TARGET)
	thorq --add ./$(TARGET) -v
