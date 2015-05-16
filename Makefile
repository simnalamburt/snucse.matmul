TARGET=matmul
OBJS=main.o timers.o

CC=gcc
CFLAGS=-g -O3 -Wall -Wextra -std=c99
LDFLAGS=-pthread

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(LDFLAGS) $(OBJS) -o $@

main.o: CFLAGS += -DTHREAD_COUNT=4

%.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $<

clean:
	rm -f $(TARGET) *.o task_*.stderr task_*.stdout

run: $(TARGET)
	thorq --add ./$(TARGET) -v
