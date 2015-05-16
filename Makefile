TARGET=matmul
OBJS=main.o

CC=gcc
CFLAGS=-O3 -Wall -Wextra -std=c99
LDFLAGS=-lpthread -lrt

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(LDFLAGS) $(OBJS) -o $@

clean:
	rm -f $(TARGET) *.o

run: $(TARGET)
	thorq --add ./$(TARGET) -v
