all: main.cc
	g++ -O3 -Wall -Wextra -std=c++0x $^ -lOpenCL -lrt -o bin

run: bin
	@thorq --add bin -n10000
