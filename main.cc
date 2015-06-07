#include <iostream>
#include <limits>
#include <cmath>
#include "mpi.h"


//
// Dead-Simple MPI wrapper
//
class mpi {
  int _rank, _size;
public:
  mpi(int &argc, char **&argv) {
    MPI_Init(&argc, &argv);
    MPI_Comm_size(MPI_COMM_WORLD, &_size);
    MPI_Comm_rank(MPI_COMM_WORLD, &_rank);
  }

  ~mpi() {
    MPI_Finalize();
  }

  int rank() const { return _rank; }
  int size() const { return _size; }

  bool root() const { return rank() == 0; }
};


//
// Entry point
//
int main(int argc, char **argv) {
  using namespace std;
  const mpi mpi(argc, argv);

  int n;
  if (mpi.root()) {
    cout << "Enter a number: " << flush;
    cin >> n;
  }

  MPI_Bcast(&n, 1, MPI_INT, 0, MPI_COMM_WORLD);

  cout << "(" << mpi.rank() << "/" << mpi.size() << ") : " << n << endl;

  int sum;
  MPI_Reduce(&n, &sum, 1, MPI_INT, MPI_SUM, 0, MPI_COMM_WORLD);

  if (mpi.root()) {
    cout << "Sum = " << sum << endl;
  }

  return 0;
}
