all: main.cc
	mpic++ -O3 -Wall -Wextra -std=c++0x $^ -o bin

run: all
	mpirun -n 4 ./bin
