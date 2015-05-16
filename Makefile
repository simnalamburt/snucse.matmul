TARGET=bin

CC=gcc
CFLAGS=-O3 -Wall -Wextra -std=c99
LDFLAGS=-lpthread -lrt

all: main.c
	$(CC) $(CFLAGS) main.c $(LDFLAGS) -o $(TARGET)

clean:
	rm -f $(TARGET)

run: $(TARGET)
	thorq --add ./$(TARGET) -v
