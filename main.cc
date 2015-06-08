#include <iostream>
#include <memory>
#include <chrono>
#include "mpi.h"

using namespace std;
using namespace chrono;


namespace {
  template<typename T> bool validate(const T&, const T&, const T&, size_t);

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
}


//
// Entry point
//
int main(int argc, char **argv) {
  const mpi mpi(argc, argv);

  size_t width = 4096;

  // Root-Only variables
  bool validation = false;
  if (mpi.root()) {
    //
    // Parse argc/argv
    //
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

  // Initialize buffer
  auto lhs    = unique_ptr<float[]>(new float[width*width]);
  auto rhs    = unique_ptr<float[]>(new float[width*width]);
  auto result = unique_ptr<float[]>(new float[width*width]);
  for (size_t i = 0; i < width*width; ++i) { lhs[i] = rhs[i] = float(i) + 1.0f; }

  // Start timer
  auto begin = system_clock::now();

  // Beginning point
  MPI_Barrier(MPI_COMM_WORLD);

  cout << "(" << mpi.rank() << "/" << mpi.size() << ") : " << width << endl;

  int sum;
  MPI_Reduce(&width, &sum, 1, MPI_INT, MPI_SUM, 0, MPI_COMM_WORLD);

  // Stop timer
  auto end = system_clock::now();
  if (mpi.root()) {
    cout << "\nTime elapsed: " << duration<double>(end - begin).count() << "s\n" << endl;
  }

  //
  // Validation
  //
  bool valid = true;
  if (validation) {
    cout << "Validating";
    valid = validate(lhs, rhs, result, width);
    cout << (valid ? "OK" : "Failed") << endl;
  }
  return !valid;
}


namespace {
  //
  // Error handling
  //
  template<typename T>
  bool validate(const T &lhs, const T &rhs, const T &result, size_t width) {
    cout << ' ';
    for (size_t i = 0; i < width; ++i) {
      if (i%64 == 63) { cout << "\033[1D.."; }
      if (i%4 == 3) {
        char rotate[] = {'/', '-', '\\', '|'};
        cout << "\033[1D" << rotate[i/4%4] << flush;
      }

      for (size_t j = 0; j < width; ++j) {
        float sum = 0;
        for (size_t k = 0; k < width; ++k) { sum += lhs[i*width + k]*rhs[k*width + j]; }

        if (result[i*width + j] != sum) { return false; }
      }
    }
    cout << "\033[1D. ";
    return true;
  }
}
