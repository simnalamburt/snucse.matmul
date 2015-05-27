all: main.cc
	g++ -O3 -Wall -Wextra -std=c++0x -fopenmp $^ -o bin

run: all
	@thorq --add bin
