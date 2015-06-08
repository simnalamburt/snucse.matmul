all: main.cc
	mpic++ -O3 -Wall -Wextra -std=c++0x $^ -o bin

run: all
	thorq --add --mode mpi --slots 16 --nodes 4 bin
