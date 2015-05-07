TARGET=mat_mul
OBJS=mat_mul.o timers.o

CC=gcc
CFLAGS=-g -O3 -Wall -Wextra -std=c99
LDFLAGS=-pthread

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(LDFLAGS) $^ -o $@

clean:
	rm -f $(TARGET) *.o task_*.stderr task_*.stdout

run: $(TARGET)
	thorq --add ./mat_mul 1  813
	thorq --add ./mat_mul 2  1024
	thorq --add ./mat_mul 3  1172
	thorq --add ./mat_mul 4  1290
	thorq --add ./mat_mul 5  1390
	thorq --add ./mat_mul 6  1477
	thorq --add ./mat_mul 7  1555
	thorq --add ./mat_mul 8  1625
	thorq --add ./mat_mul 9  1691
	thorq --add ./mat_mul 10 1751
	thorq --add ./mat_mul 11 1808
	thorq --add ./mat_mul 12 1861
	thorq --add ./mat_mul 13 1911
	thorq --add ./mat_mul 14 1959
	thorq --add ./mat_mul 15 2004
	thorq --add ./mat_mul 16 2048
