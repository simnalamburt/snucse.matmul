TARGET=matmul
OBJS=main.o timers.o

CC=gcc
CFLAGS=-O3 -Wall -Wextra -std=c99
LDFLAGS=-pthread

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(LDFLAGS) $(OBJS) -o $@

clean:
	rm -f $(TARGET) *.o

run: $(TARGET)
	thorq --add ./$(TARGET) -v
