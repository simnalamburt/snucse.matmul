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


using namespace std;


//
// Root-Only variables
//


//
// Entry point
//
int main(int argc, char **argv) {
  const mpi mpi(argc, argv);

  size_t width = 4096;

  // Root-Only variables
  bool validation = false;

  if (mpi.root()) {
    for (int opt; (opt = getopt(argc, argv, "n:vh")) != -1;) {
      switch (opt) {
      case 'n':
        width = stoul(string(optarg));
        break;

      case 'v':
        validation = true;
        break;

      case 'h':
      default:
        cout << R"(Parallel Matrix Multiplier

USAGE: )" << argv[0] << R"( [-pvh]

OPTIONS:
  -n <N>    Change width of a matrix (N*N). Default size is 2048.
  -v        Validate calculate results.
  -h        Print this page.
)";
      }
    }
  }

  MPI_Bcast(&width, 1, MPI_INT, 0, MPI_COMM_WORLD);

  cout << "(" << mpi.rank() << "/" << mpi.size() << ") : " << width << endl;

  int sum;
  MPI_Reduce(&width, &sum, 1, MPI_INT, MPI_SUM, 0, MPI_COMM_WORLD);

  if (mpi.root()) {
    cout << "Sum = " << sum << endl;
  }

  return 0;
}
