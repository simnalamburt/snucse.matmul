all: main.cc
	g++ -O3 -Wall -Wextra -std=c++0x -fopenmp $^ -o bin

run: bin
	@thorq --add --device gpu bin -n10000
