TARGET=bin

CC=g++
CFLAGS=-O3 -Wall -Wextra -std=c++0x
LDFLAGS=-lOpenCL -lrt

all: main.cc
	$(CC) $(CFLAGS) $^ $(LDFLAGS) -o $(TARGET)

clean:
	rm -f $(TARGET)

run: $(TARGET)
	thorq --add ./$(TARGET) -v
